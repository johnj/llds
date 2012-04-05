#define _GNU_SOURCE 1 /* for CPU_* macros in sched.h */
#include <stdio.h>
#include <rbtree.h>
#include <stdint.h>

#include "forrest.h"
#include "tests.h"

#include <sched.h>
#include <pthread.h>

pthread_rwlock_t rb_t_wrl = PTHREAD_RWLOCK_INITIALIZER;

typedef struct __tmp_llds_entry {
    uint64_t key;
    void *val;
    int vlen;
    struct rb_node rb;
} t_llds_entry;

struct rb_root rt = RB_ROOT;

static inline t_llds_entry *t_llds_search_by_key(struct rb_root *tn_root, uint64_t search_key) {
  struct rb_node *next;
  t_llds_entry *tn = NULL;

  next = tn_root->rb_node;
  while(next) {
    tn = rb_entry(next, t_llds_entry, rb);

    if(search_key < tn->key) {
      next = next->rb_left;
    } else if (search_key > tn->key) {
      next = next->rb_right;
    } else {
      return tn;
    }
  }

  return NULL;
}

static inline t_llds_entry *t_alloc_llds_entry(void) {
  t_llds_entry *ret;
  ret = malloc(sizeof(t_llds_entry));
  return ret;
}

static void t_llds_insert_by_key(struct rb_root *ver_root, t_llds_entry *ins_tn) {
  struct rb_node **uncle = &ver_root->rb_node;
  struct rb_node *parent = NULL;
  t_llds_entry *tn;

  while(*uncle!=NULL) {
    parent = *uncle;
    tn = rb_entry(parent, t_llds_entry, rb);

    if(ins_tn->key < tn->key) {
      uncle = &parent->rb_left;
    } else if(ins_tn->key > tn->key) {
      uncle = &parent->rb_right;
    } else {
      return;
    }
  }
  rb_link_node(&ins_tn->rb, parent, uncle);
  rb_insert_color(&ins_tn->rb, ver_root);
}

void *rb_t_thr(void *a) {
  uint64_t i, start, end;
  rb_t_attrs *attrs = (rb_t_attrs *)a;
  int mode = attrs->mode;
  char idx_key[128], val[128];
  t_llds_entry *ins_elem, *res;
  
  start = attrs->tno * attrs->items_per_loop;
  end = start + attrs->items_per_loop;

  fprintf(stderr, "helo from %s thread %d\n", mode ? "reader" : "writer", attrs->tno);

  for(i=start; i<end; i++) {
    int l = snprintf(idx_key, 128, "key%lu", i);
    uint64_t key = ft_MurmurHash64A(idx_key, l, 9);
    pthread_rwlock_rdlock(&rb_t_wrl);
    res = t_llds_search_by_key(&rt, key);
    pthread_rwlock_unlock(&rb_t_wrl);

    if(res && !mode) {
	continue;
    }

    if(!mode) {
      int vl = snprintf(val, 128, "%lu", i);

      ins_elem = t_alloc_llds_entry();
      ins_elem->key = key;

      ins_elem->val = malloc(vl + 1);
      memcpy(ins_elem->val, val, vl + 1);

      pthread_rwlock_wrlock(&rb_t_wrl);
      t_llds_insert_by_key(&rt, ins_elem);
      pthread_rwlock_unlock(&rb_t_wrl);
    } 
  }
}

int main(int argc, char **argv) {
  uint64_t i, items_per_thread = NO_OF_ITEMS;
  int threads = 1;
  pthread_t t_th[MAX_THREADS];

  if(argc > 1) {
    threads = atoi(argv[1]);
    threads = threads > MAX_THREADS ? MAX_THREADS : threads;
  }
  if(argc > 2) {
    items_per_thread = atoi(argv[2]);
  }

  if(threads > 1) {
    items_per_thread = items_per_thread / threads;
  }

  fprintf(stderr, "benchmarking with %d threads (%lu items per thread)\n", threads, items_per_thread);

  print_resource_usage();

  /* writers */
  for(i=0; i<threads; i++) {
    rb_t_attrs *attrs = malloc(sizeof(rb_t_attrs)); /* unfreed */
    attrs->tno = i;
    attrs->items_per_loop = items_per_thread;
    attrs->mode = 0;

    if(pthread_create(&t_th[i], NULL, rb_t_thr, (void*)attrs)) {
      fprintf(stderr, "thread creation failed:\n");
      perror("pthread_create");
    }
  }

  for(i=0; i<threads; i++) {
    pthread_join(t_th[i], NULL);
  }

  print_resource_usage();

  /* readers */
  for(i=0; i<threads; i++) {
    rb_t_attrs *attrs = malloc(sizeof(rb_t_attrs)); /* unfreed */
    attrs->tno = i;
    attrs->items_per_loop = items_per_thread;
    attrs->mode = 1;

    if(pthread_create(&t_th[i], NULL, rb_t_thr, (void*)attrs)) {
      fprintf(stderr, "thread creation failed:\n");
      perror("pthread_create");
    }
  }

  for(i=0; i<threads; i++) {
    pthread_join(t_th[i], NULL);
  }

  return 0;
}
