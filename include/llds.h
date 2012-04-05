#ifndef __K_LLDS_H
#define __K_LLDS_H

#include "llds_common.h"

#define CHRDEV_MJR_NUM 834
#define LLDS_IOCTL_SET_ENTRY _IOR(CHRDEV_MJR_NUM, 1, llds_result_ent)
#define LLDS_IOCTL_SEARCH _IOR(CHRDEV_MJR_NUM, 2, llds_result_ent)
#define LLDS_IOCTL_EXPIRE_DOC_ID _IOR(CHRDEV_MJR_NUM, 3, llds_result_ent)
#define LLDS_IOCTL_RMTREE _IOR(CHRDEV_MJR_NUM, 4, llds_result_ent)
#define LLDS_CDEV_NAME "llds"

#endif
