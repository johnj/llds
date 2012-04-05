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

#define SUCCESS 0
#define FAILURE 1

typedef struct __kllds_entry {
    uint64_t key;
    uint64_t *doc_ids;
    int ndoc_ids;
    struct rb_node rb;
} kllds_entry;

struct rb_root kllds_rbtree = RB_ROOT;
static struct kmem_cache *tmp_kllds_cache;

uint64_t *expired_doc_ids = NULL;
uint64_t *realloced_expired_doc_ids;
int nexpired_doc_ids = 0L;

/* this is only used for tree set ops */
DEFINE_SPINLOCK(jj_wr_spinlock);
/* expired docs write lock */
DEFINE_SPINLOCK(expired_doft_wr_spinlock);

static __always_inline kllds_entry *alloc_kllds_cache(void) {
    kllds_entry *ret;
    ret = kmem_cache_zalloc(tmp_kllds_cache, GFP_ATOMIC); /* jj: do we really need to 0 out here? */
    return ret;
}

static __always_inline kllds_entry *kllds_search_by_key(struct rb_root *tn_root, uint64_t search_key) {
    struct rb_node *next;
    kllds_entry *tn = NULL;

    rcu_read_lock();

    next = tn_root->rb_node;
    while(next) {
	tn = rb_entry(next, kllds_entry, rb);

	if(search_key < tn->key) {
	  next = next->rb_left;
	} else if (search_key > tn->key) {
	  next = next->rb_right;
	} else {
	    return tn;
	}
    }
    rcu_read_unlock();

    return NULL;
}

static void kllds_insert_by_key(struct rb_root *ver_root, kllds_entry *ins_tn) {
    struct rb_node **uncle = &ver_root->rb_node;
    struct rb_node *parent = NULL;
    kllds_entry *tn;

    spin_lock(&jj_wr_spinlock);
    while(*uncle!=NULL) {
	parent = *uncle;
	tn = rb_entry(parent, kllds_entry, rb);

	if(ins_tn->key < tn->key) {
	  uncle = &parent->rb_left;
	} else if(ins_tn->key > tn->key) {
	  uncle = &parent->rb_right;
	} else {
	    spin_unlock(&jj_wr_spinlock);
	    return;
	}
    }
    rb_link_node(&ins_tn->rb, parent, uncle);
    rb_insert_color(&ins_tn->rb, ver_root);
    spin_unlock(&jj_wr_spinlock);
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

long handle_ioctl_ops(struct file *f, unsigned int ioctl_opcode, unsigned long uptr) {
    struct ft_result_ent __user *jj = (struct ft_result_ent __user *)uptr;
    int doci;
    kllds_entry *res_elem ____cacheline_aligned_in_smp;
    uint64_t *new_doc_ids;

    /* jj: I'm not happy with how we handle expired docs, this needs to be purged by some process or the mod needs to be rm/ins again to maintain efficiency */
    if(ioctl_opcode==LLDS_IOCTL_EXPIRE_DOC_ID) {
	if(!nexpired_doc_ids) {
                spin_lock(&expired_doft_wr_spinlock);
		expired_doc_ids = kzalloc(sizeof(uint64_t), GFP_ATOMIC);
		expired_doc_ids[0] = jj->doc_id;
		nexpired_doc_ids = 1L;
                spin_unlock(&expired_doft_wr_spinlock);
	} else {
		for(doci=0; doci<nexpired_doc_ids; doci++) {
			if(jj->doc_id==expired_doc_ids[doci]) {
				return SUCCESS;
			}
		}
                spin_lock(&expired_doft_wr_spinlock);
		++nexpired_doc_ids;
		realloced_expired_doc_ids = krealloc(expired_doc_ids, sizeof(uint64_t) * (nexpired_doc_ids + 1), GFP_ATOMIC);
		expired_doc_ids = realloced_expired_doc_ids;
		expired_doc_ids[nexpired_doc_ids-1] = jj->doc_id;
                spin_unlock(&expired_doft_wr_spinlock);
	}
	return SUCCESS;
    }

    jj->result = 0UL;

    res_elem = kllds_search_by_key(&kllds_rbtree, jj->key);

    if(ioctl_opcode==LLDS_IOCTL_SEARCH) {
	if(!res_elem) {
	    return FAILURE;
	}
	jj->result = 1UL;
	/*jj->found_doc_ids = res_elem->doc_ids;*/
	copy_to_user(jj->found_doc_ids, res_elem->doc_ids, sizeof(uint64_t) * res_elem->ndoc_ids);
	jj->nfound_doc_ids = res_elem->ndoc_ids;
	return SUCCESS;
    }

    /*if(ioctl_opcode==LLDS_IOCTL_SET_ENTRY) { case optimized out */
    if(res_elem) {
	for(doci=0; doci<res_elem->ndoc_ids; doci++) {
	    if(jj->doc_id==res_elem->doc_ids[doci]) {
		/* found this docid, no need to do anything further */
		return SUCCESS;
	    }
	}
	++res_elem->ndoc_ids;
	new_doc_ids = krealloc(res_elem->doc_ids, sizeof(uint64_t) * (res_elem->ndoc_ids), GFP_ATOMIC);
	if(!new_doc_ids) {
		return -ENOMEM;
	}
	res_elem->doc_ids = new_doc_ids;
	res_elem->doc_ids[res_elem->ndoc_ids-1] = jj->doc_id;
    } else {
	kllds_entry *ins_elem ____cacheline_aligned_in_smp;
	/* new entry */
	ins_elem = alloc_kllds_cache();
	ins_elem->key = jj->key;
	ins_elem->doc_ids = kzalloc(sizeof(uint64_t), GFP_ATOMIC);

	/* we don't need to copy primitive mem addrs from userspace, the following code is left as a reference for any future work that might be need to copy more than primitives */
	//get_user(new_doc_id, &jj->doc_id);
	//ins_elem->doc_ids[0] = new_doc_id;
	ins_elem->doc_ids[0] = jj->doc_id;
	ins_elem->ndoc_ids = 1UL;
	kllds_insert_by_key(&kllds_rbtree, ins_elem);
    }
    /*}*/

    /* jj: feel free to bring back this switch when you have more than 2 iocodes to deal with, I just wanted to optimize the paths/branch prediction for the 2 ops supported initially */
    /*switch(ioctl_opcode) {
      case LLDS_IOCTL_SEARCH:
      elem = kllds_search_by_key(&kllds_rbtree, (uint64_t)jj->key);
      case LLDS_IOCTL_SET_ENTRY:
      elem = alloc_kllds_cache();
      elem->key = (uint64_t)jj->key;
      kllds_insert_by_key(&kllds_rbtree, elem);
      break;
      }*/
    return SUCCESS;
}

struct file_operations fops = {
    .read = handle_dev_read,
    .write = handle_dev_write,
#ifdef HAVE_UNLOCKED_IOCTL
    .unlocked_ioctl = handle_ioctl_ops,
#else
#error "seriously you shouldn't be running kllds with unlocked ioctl...get a real kernel, this is a feeble attempt to stop you from an epic fail! <3 jj"
    .ioctl = handle_ioctl_ops,
#endif
    .open = handle_dev_open,
    .release = handle_dev_release,
};

static int __init kllds_init_module(void) {
    int ret_val;
    ret_val = register_chrdev(CHRDEV_MJR_NUM, LLDS_CDEV_NAME, &fops);

    printk(KERN_INFO "Make sure you execute the following:\n# mknod /dev/%s c %d 0\nbefore trying to work with libforrest (unless it exists)!\n", LLDS_CDEV_NAME, CHRDEV_MJR_NUM);
    printk(KERN_INFO "Page size: %lu!\n", PAGE_SIZE);

    tmp_kllds_cache = KMEM_CACHE(__kllds_entry, SLAB_HWCACHE_ALIGN);
    return 0;
}

static void __exit kllds_cleanup_module(void) {
    unregister_chrdev(CHRDEV_MJR_NUM, LLDS_CDEV_NAME);
    kmem_cache_destroy(tmp_kllds_cache);
    kfree(expired_doc_ids);
}

module_init(kllds_init_module)
module_exit(kllds_cleanup_module)
