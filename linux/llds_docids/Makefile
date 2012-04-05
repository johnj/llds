obj-m += llds.o

all:
	make -C /lib/modules/`uname -r`/build M=`pwd`

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
