#include <stdio.h>
#include <sys/types.h>
#include "forrest.h"

void main() {
	long i = 0;
	FORREST *cs;
	cs = forrest_alloc();

	char *idx_key = "key2";
	char *val = "llds is new";

	uint64_t key = ft_MurmurHash64A(idx_key, strlen(idx_key), 9);
	fprintf(stderr, "[set] key: %s mmhash: %lu\n", idx_key, key);
	forrest_insert_key(cs, key, val, strlen(val) + 1);
	llds_result_ent *res = forrest_get_key(cs, key);
	fprintf(stderr, "[get] %s\n", (char *)res->val);
}
