
knv :=$(shell uname -r)
KERNELDIR :=/lib/modules/$(knv)/build
PWD :=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules 

clean:
	make -C $(KERNELDIR) M=$(PWD) modules clean
	rm -rf modules.order

obj-m		+= spidev-hi3593.o


