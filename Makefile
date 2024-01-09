# Set the path to the Kernel build utils.
KBUILD=/lib/modules/$(shell uname -r)/build/

test-objs := main.o

obj-m += test.o

default:
	$(MAKE) -C $(KBUILD) M=$(PWD) V=$(VERBOSE) modules
