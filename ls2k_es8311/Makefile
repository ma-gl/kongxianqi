
# 1. 使用不同的开发板内核时, 一定要修改KERN_DIR
# 2. KERN_DIR中的内核要事先配置、编译, 为了能编译内核, 要先设置下列环境变量:
# 2.1 ARCH,          比如: export ARCH=arm64
# 2.2 CROSS_COMPILE, 比如: export CROSS_COMPILE=aarch64-linux-gnu-
# 2.3 PATH,          比如: export PATH=$PATH:/home/book/100ask_roc-rk3399-pc/ToolChain-6.3.1/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin 
# export PATH=$PATH:/media/tronlong/AM335x/work/loongsontest_driver/gcc-4.9.3-64-gnu/opt/gcc-4.9.3-64-gnu/bin
#编译内核 make ARCH=mips CROSS_COMPILE=mips64el-linux- -j 16

#KERN_DIR = /media/tronlong/AM335x/work/loongsontest_driver/linux-3.10
knv :=$(shell uname -r)
KERNELDIR :=/lib/modules/$(knv)/build
PWD :=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules 

clean:
	make -C $(KERNELDIR) M=$(PWD) modules clean
	rm -rf modules.order


# 参考内核源码drivers/char/ipmi/Makefile
# 要想把a.c, b.c编译成ab.ko, 可以这样指定:
# ab-y := a.o b.o
# obj-m += ab.o

obj-m		+= ls2k_es8311.o


