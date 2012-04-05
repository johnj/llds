#include <stdio.h>
#include <stdint.h>

#include "forrest.h"
#include "tests.h"

#include <sched.h>
#include <pthread.h>

void *rb_t_thr(void *a) {
  uint64_t i, start, end;
  FORREST *f;
  llds_result_ent *res;
  rb_t_attrs *attrs = (rb_t_attrs *)a;
  int mode = attrs->mode;
  char idx_key[128], val[128];
  
  f = forrest_alloc();

  start = attrs->tno * attrs->items_per_loop;
  end = start + attrs->items_per_loop;

  fprintf(stderr, "helo from %s thread %d\n", mode ? "reader" : "writer", attrs->tno + 1);

  for(i=start; i<end; i++) {
    int l = snprintf(idx_key, 128, "key%lu", i);
    uint64_t key = ft_MurmurHash64A(idx_key, l, 9);

    if(!mode) {
      int vl = snprintf(val, 128, "%lu", i);
      res = forrest_insert_key(f, key, val, vl + 1);
    } else {
      res = forrest_get_key(f, key);
      free(res->val);
    }

    /* the copy below doesn't need to happen for llds (it is copied later by llds from userspace) but it is here keep the benchmark as equivalent as possible
    res->val = malloc(vl + 1);
    memcpy(res->val, val, vl + 1);

    free(res->val);*/

    free(res);
  }

  forrest_cleanup(f);
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
