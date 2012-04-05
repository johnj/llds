#ifndef __K_LLDS_COMMON_H
#define __K_LLDS_COMMON_H

/* yeah...if these structs become unaligned I'll personally git-blame you to the ends of the earth */
struct ft_result_ent {
	unsigned long long key; /* 8 */
	uint64_t doc_id; /* 8 */
	uint64_t *found_doc_ids; /* 8 */
	int result; /* 4 */
	unsigned int nfound_doc_ids; /* 4 */
};

#endif
