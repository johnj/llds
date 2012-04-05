#include <stdio.h>
#include <stdlib.h>
#include "forrest.h"

/* http://sites.google.com/site/murmurhash/ this hash func is seriously *AWESOME* */
inline uint64_t ft_MurmurHash64A(const void * key, int len, unsigned int seed)
{
	const uint64_t m = 0xc6a4a7935bd1e995;
	const int r = 47;

	uint64_t h = seed ^ (len * m);

	const uint64_t * data = (const uint64_t *)key;
	const uint64_t * end = data + (len/8);

	while(data != end)
	{
		uint64_t k = *data++;

		k *= m; 
		k ^= k >> r; 
		k *= m; 
		
		h ^= k;
		h *= m; 
	}

	const unsigned char * data2 = (const unsigned char*)data;

	switch(len & 7)
	{
	case 7: h ^= (uint64_t)(data2[6]) << 48;
	case 6: h ^= (uint64_t)(data2[5]) << 40;
	case 5: h ^= (uint64_t)(data2[4]) << 32;
	case 4: h ^= (uint64_t)(data2[3]) << 24;
	case 3: h ^= (uint64_t)(data2[2]) << 16;
	case 2: h ^= (uint64_t)(data2[1]) << 8;
	case 1: h ^= (uint64_t)(data2[0]);
	        h *= m;
	};
 
	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

/* return error or something more suitable for rel */
FORREST *forrest_alloc() {
	FORREST *cs = malloc(sizeof(FORREST));
	char full_dev_name[128] = "";
	snprintf(full_dev_name, 128, "/dev/%s", LLDS_CDEV_NAME);
	cs->fd = open(full_dev_name, O_RDONLY);
	return cs;
}

llds_result_ent *forrest_insert_key(FORREST *cs, uint64_t key, void *val, int vlen) {
	llds_result_ent *res = malloc(sizeof(llds_result_ent));
	
	if(!res) {
		perror("malloc");
		return NULL;
	}

	res->key = key;
	res->result = -1;
	res->val = val;
	res->vlen = vlen;
	ioctl(cs->fd, LLDS_IOCTL_SET_ENTRY, res);

	return res;
}

llds_result_ent *forrest_get_key(FORREST *cs, uint64_t key) {
	llds_result_ent *res = malloc(sizeof(llds_result_ent));
	
	if(!res) {
		perror("malloc");
		return NULL;
	}

	res->key = key;
	res->result = -1;
	res->val = malloc(FORREST_MAX_VAL_LEN);
	ioctl(cs->fd, LLDS_IOCTL_SEARCH, res);
	return res;
}

llds_result_ent *forrest_rm_tree(FORREST *cs) {
	llds_result_ent *res = malloc(sizeof(llds_result_ent));
	
	if(!res) {
		perror("malloc");
		return NULL;
	}

	res->key = 0L;
	res->result = -1;
	res->val = NULL;
	ioctl(cs->fd, LLDS_IOCTL_RMTREE, res);
	return res;
}

void *forrest_cleanup(FORREST *cs) {
	if(cs) { return NULL; }

	if(cs->fd) {
		close(cs->fd);
	}

	free(cs);

	return SUCCESS;
}
