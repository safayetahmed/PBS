SRCS	= LAMbS_mo.c LAMbS_molookup.c LAMbS_mostat.c LAMbS_governor_main.c
TESTSRC = freq_change_test.c

obj-m += LAMbS_governor.o
obj-m += freq_change_test.o

LAMbS_governor-objs := $(SRCS:.c=.o)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	$(RM) -r Module.markers modules.order Module.symvers .tmp_versions/
	
