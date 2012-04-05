/* LICENSE - see LICENSE file included in this source distribution */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/mount.h>
#include <linux/fs.h>
#include <linux/vfs.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/rbtree.h>
#include <linux/slab.h>

#include "kmodllds.h"
#include "../../include/llds_common.h"
#include "../../include/llds.h"

MODULE_LICENSE("BSD");

#define SUCCESS 0
#define FAILURE 1

typedef struct __kllds_entry {
  uint64_t key;
  void *val;
  int vlen;
  struct rb_node rb;
} kllds_entry;

struct rb_root kllds_rbtree = RB_ROOT;
static struct kmem_cache *tmp_kllds_cache;

/* this is only used for tree set ops */
rwlock_t kllds_rwlck = RW_LOCK_UNLOCKED;

static __always_inline kllds_entry *alloc_kllds_cache(void) {
  kllds_entry *ret;
  ret = kmem_cache_zalloc(tmp_kllds_cache, GFP_ATOMIC); /* jj: do we really need to 0 out here? */
  return ret;
}

static void kllds_remove_rbtree(void) {
  kllds_entry *elem;
  struct rb_node *next;

  write_lock(&kllds_rwlck);
  while ((next = rb_first(&kllds_rbtree))) {
    elem = rb_entry(next, kllds_entry, rb);
    rb_erase(next, &kllds_rbtree);
    kfree(elem->val);
    kmem_cache_free(tmp_kllds_cache, elem);
  }
  write_unlock(&kllds_rwlck);
}

static __always_inline kllds_entry *kllds_search_by_key(struct rb_root *tn_root, uint64_t search_key) {
  struct rb_node *next;
  kllds_entry *tn = NULL;

  read_lock(&kllds_rwlck);

  next = tn_root->rb_node;
  while(next) {
    tn = rb_entry(next, kllds_entry, rb);

    if(search_key < tn->key) {
      next = next->rb_left;
    } else if (search_key > tn->key) {
      next = next->rb_right;
    } else {
      read_unlock(&kllds_rwlck);
      return tn;
    }
  }

  read_unlock(&kllds_rwlck);

  return NULL;
}

static void kllds_insert_by_key(struct rb_root *ver_root, kllds_entry *ins_tn) {
  struct rb_node **uncle = &ver_root->rb_node;
  struct rb_node *parent = NULL;
  kllds_entry *tn;

  write_lock(&kllds_rwlck);
  while(*uncle!=NULL) {
    parent = *uncle;
    tn = rb_entry(parent, kllds_entry, rb);

    if(ins_tn->key < tn->key) {
      uncle = &parent->rb_left;
    } else if(ins_tn->key > tn->key) {
      uncle = &parent->rb_right;
    } else {
      write_unlock(&kllds_rwlck);
      return;
    }
  }
  rb_link_node(&ins_tn->rb, parent, uncle);
  rb_insert_color(&ins_tn->rb, ver_root);
  write_unlock(&kllds_rwlck);
}

static int handle_dev_open(struct inode *inode, struct file *file) {
  try_module_get(THIS_MODULE);
  return SUCCESS;
}

static int handle_dev_release(struct inode *inode, struct file *file) {
  module_put(THIS_MODULE);
  return SUCCESS;
}

static ssize_t handle_dev_read(struct file *fp, char __user * buf, size_t len, loff_t * offset) {
  /* NOOP right now, we need to simply apply a rcu read lock here and then start reading the tree...providing it in some sort of format back to the process which wanted to read...needs to be reset per open() */
  return len;
}

static ssize_t handle_dev_write(struct file *fp, const char __user * buf, size_t len, loff_t * offset) {
  /* NOOP right now, needs to take a "line" of data and figure out the hash etc */
  return len;
}

long handle_ioctl_ops(struct file *f, unsigned int ioctl_opcode, unsigned long up) {
  kllds_entry *res_elem ____cacheline_aligned_in_smp;
  llds_result_ent __user *uptr = (llds_result_ent __user *)up;

  if(ioctl_opcode==LLDS_IOCTL_RMTREE) {
    printk(KERN_INFO "llds: removing tree\n");
    kllds_remove_rbtree();
  }

  res_elem = kllds_search_by_key(&kllds_rbtree, uptr->key);

  if(ioctl_opcode==LLDS_IOCTL_SEARCH) {
    if(!res_elem) {
      return FAILURE;
    }
    copy_to_user(uptr->val, res_elem->val, res_elem->vlen);
    return SUCCESS;
  }

  /*if(ioctl_opcode==LLDS_IOCTL_SET_ENTRY) { case optimized out */
  if(res_elem) {
    /* find, and update value */
  } else {
    kllds_entry *ins_elem ____cacheline_aligned_in_smp;
    /* new entry */
    ins_elem = alloc_kllds_cache();
    ins_elem->key = uptr->key;
    ins_elem->vlen = uptr->vlen;

    ins_elem->val = kzalloc(uptr->vlen, GFP_ATOMIC);
    copy_from_user(ins_elem->val, uptr->val, uptr->vlen);

    kllds_insert_by_key(&kllds_rbtree, ins_elem);
  }
  /*}*/

  return SUCCESS;
}

struct file_operations fops = {
  .read = handle_dev_read,
  .write = handle_dev_write,
#ifdef HAVE_UNLOCKED_IOCTL
  .unlocked_ioctl = handle_ioctl_ops,
#else
#error "you shouldn't be running llds in a kernel without unlocked ioctl"
  .ioctl = handle_ioctl_ops,
#endif
  .open = handle_dev_open,
  .release = handle_dev_release,
};

static int __init kllds_init_module(void) {
  int ret_val;
  ret_val = register_chrdev(CHRDEV_MJR_NUM, LLDS_CDEV_NAME, &fops);

  printk(KERN_INFO "llds: make sure you execute the following:\nllds: # mknod /dev/%s c %d 0\nllds: before attempting to work with libforrest (unless it exists)!\n", LLDS_CDEV_NAME, CHRDEV_MJR_NUM);
  printk(KERN_INFO "llds: page size is %lu bytes\n", PAGE_SIZE);

  tmp_kllds_cache = KMEM_CACHE(__kllds_entry, SLAB_HWCACHE_ALIGN);
  return 0;
}

static void __exit kllds_cleanup_module(void) {
  kmem_cache_destroy(tmp_kllds_cache);
  unregister_chrdev(CHRDEV_MJR_NUM, LLDS_CDEV_NAME);
}

module_init(kllds_init_module)
module_exit(kllds_cleanup_module)
