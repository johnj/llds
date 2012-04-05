.PHONY: all

all:
	gcc -ggdb -O2 -o rb_userspace -I. -I../../include -I../../libforrest/src -lforrest rbtree.c rb_userspace.c tests.c -lproc -pthread
	gcc -ggdb -O2 -o rb_llds -I. -I../../include -I../../libforrest/src -lforrest rb_llds.c tests.c -lproc -pthread
	gcc -ggdb -O2 -o rb_rmtree -I. -I../../include -I../../libforrest/src -lforrest rb_rmtree.c 
