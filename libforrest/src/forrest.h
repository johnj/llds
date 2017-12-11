#ifndef __FORREST_H
#define __FORREST_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <inttypes.h>
#include <sys/time.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "llds.h"

#define SUCCESS 0  /* The request was successful. */

typedef struct __forrest {
	int fd;
} forrest;

typedef forrest FORREST;

FORREST *forrest_alloc();
uint64_t ft_MurmurHash64A(const void *key, int len, unsigned int seed);
llds_result_ent *forrest_insert_key(FORREST *cs, uint64_t key, void *val, int vlen);
llds_result_ent *forrest_get_key(FORREST *cs, uint64_t key);
llds_result_ent *forrest_rm_tree(FORREST *cs);

#ifndef FORREST_MAX_VAL_LEN
#define FORREST_MAX_VAL_LEN 4096L
#endif

#endif
