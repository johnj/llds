obj-m += llds.o

all:
	make -C /lib/modules/`uname -r`/build M=`pwd` KBUILD_VERBOSE=1 CONFIG_DEBUG_INFO=1 modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	make -C /lib/modules/`uname -r`/build M=`pwd` KBUILD_VERBOSE=1 CONFIG_DEBUG_INFO=1 modules_install
