#ifndef _LLDS_TEST_H
#define _LLDS_TEST_H

#include <stdint.h>

/*#define NO_OF_ITEMS 1279000000L*/
/*#define NO_OF_ITEMS 4020000000L*/
#define NO_OF_ITEMS 2003009040L

#define MAX_THREADS 24L
#define __PG_SIZE (sysconf(_SC_PAGESIZE))

typedef struct __rb_t_attrs {
  int tno;
  int mode;
  uint64_t items_per_loop;
} rb_t_attrs;

void print_resource_usage();

#endif
