#include <stdio.h>
#include <proc/readproc.h>
#include "tests.h"

void print_resource_usage() {
	struct proc_t u;
	look_up_our_self(&u);
	printf("rss usage: %lu bytes\n", u.rss * __PG_SIZE);
}
