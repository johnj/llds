#ifndef __K_LLDS_COMMON_H
#define __K_LLDS_COMMON_H

/* yeah...if these structs become unaligned I'll personally git-blame you to the ends of the earth */
typedef struct __llds_result_ent {
	uint64_t key; /* 8 */
	void *val; /* 8 */
	int vlen; /* 4 */
	int result; /* 4 */
} llds_result_ent;

#endif
