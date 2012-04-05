#ifndef __KADU_KERN_H
#define __KADU_KERN_H

#include <linux/ioctl.h>
#include <linux/lockdep.h>
#include <linux/mutex.h>

struct ioctl_res {
	unsigned long long key;
	long result;
};

extern spinlock_t rbtree_wr_spinlock;
extern spinlock_t expired_docs_wr_spinlock;

#endif
