obj-m:=cache_on.o
CURRENT_PATH := $(shell pwd)
LINUX_KERNEL := $(shell uname -r)
#LINUX_KERNEL_PATH := /lib/modules/$(shell uname -r)/build
#LINUX_KERNEL_PATH := /home/is11/zhang/linux-3.2-rc1
LINUX_KERNEL_PATH := /home/zz/zhi/update_iotlb/diy_cache_linux-3.2-rc1/linux-3.2-rc1

all: 
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
