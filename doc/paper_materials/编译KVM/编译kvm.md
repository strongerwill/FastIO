编译 KVM with vIOMMU
==========================

# 版本
    kernel version = 2.6.35
    (commit 9fe6206f400646a2322096b56c59891d530e8d51)

# 获取源代码

    git clone git://git.kernel.org/pub/scm/virt/kvm/kvm.git
    git checkout -b patch 9fe6206f400646a2322096b56c59891d530e8d51
    git am vIOMMU.patch
    

# 配置、编译

KVM.git 实际上是有包含了kvm开发中代码的一个完整Linux kernel repo。所以，kvm的配置、编译与kernel无异。

## 配置

简单起见，在默认配置(`kvm/arch/x86/configs/i386_defconfig`)的基础上增加对kvm的相应修改。

    cp -v arch/x86/configs/i386_defconfig .config
    make menuconfig
    

## 编译

    make && make modules_install && make install

-----

# 编译时遇到的问题

## Problem 1

`do_hypervisor_ -> xen_do_hypervisor`

## elf_x86_64 absent

    elf_x86_64: No such file or directory

The problem is that gcc 4.6 doesn't support anymore linker-style architecture options.  


- Solution1: `git cherry-pick --no-commit de2a8cf9`
[ref1](https://groups.google.com/forum/#!topic/linux.debian.bugs.dist/ndi_20K36xI)

- Solution2: In `arch/x86/vdso/Makefile` :

    - replace `-m elf_x86_64` by `-m64` on the line starting with  `VDSO_LDFLAGS_vdso.lds`
    - replace  `-m elf_i386` by `-m32` on the line starting with  `VDSO_LDFLAGS_vdso32.lds`

## duplicate member 'page'

    In file included from drivers/net/igbvf/ethtool.c:36:0:
    drivers/net/igbvf/igbvf.h: At top level:
    drivers/net/igbvf/igbvf.h:129:15: error: duplicate member ‘page’


### solution

修改`drivers/net/igbvf/igbvf.h`，将重复声明的结构体成员`page`注释。

    diff --git a/drivers/net/igbvf/igbvf.h b/drivers/net/igbvf/igbvf.h
    index debeee2..510c726 100644
    --- a/drivers/net/igbvf/igbvf.h
    +++ b/drivers/net/igbvf/igbvf.h
    @@ -126,7 +126,7 @@ struct igbvf_buffer {
                            unsigned int page_offset;
                    };
            };
    -       struct page *page;
    +       //struct page *page;
     };

But it looks very broken.

## 链接问题

编译到打过补丁的那部分代码时，`undefined ref`错误。目标文件`built-in.o`链接静态链接库时出错。

    arch/x86/built-in.o: In function `kvm_vm_ioctl':
    kvm_main.c:(.text+0x39b5): undefined reference to `alloc_viommu'
    kvm_main.c:(.text+0x39f6): undefined reference to `iommu_phy_addr_translate'
    kvm_main.c:(.text+0x3a5f): undefined reference to `intel_iommu_add_dev'
    kvm_main.c:(.text+0x3a69): undefined reference to `kvm_viommu_exit'
    kvm_main.c:(.text+0x3a93): undefined reference to `viommu_perform_write'
    arch/x86/built-in.o: In function `kvm_arch_destroy_vm':
    (.text+0x118d2): undefined reference to `kvm_viommu_exit'
    arch/x86/built-in.o: In function `kvm_arch_vcpu_ioctl_run':
    (.text+0x1249a): undefined reference to `viommu_mark_vmexit'
    make: *** [.tmp_vmlinux1] Error 1

浏览`kvm_main.c`源代码发现，这些函数都在`include/linux/viommu.h`中声明，为什么没有引用呢，定是`Makefile`的问题。`vIOMMU-kvm.patch`在`arch/x86/kvm/Makefile`中增加了一行，

    kvm-$(CONFIG_IOMMU_API) += $(addprefix ../../../virt/kvm/, iommu.o viommu.o)
    
根据上下文猜测，这一行是根据配置文件中`$CONFIG_IOMMU_API`的值而定义编译选项：如果`$CONFIG_IOMMU_API = y`，这一行将被展开为 `kvm-y += ..`，否则不起作用。

但，`CONFIG_IOMMU_API`并没有出现在`menuconfig`中，需要手动加上

    make CONFIG_IOMMU_API=y
    
有些文件中有用到条件编译`#ifdef CONFIG_IOMMU_API`，不知道如何在`Makefile`中宏定义，干脆直接修改一些文件好了...重新编译，问题解决。



    struct iommu_flush {
            void (*flush_context)(struct intel_iommu *iommu, u16 did, u16 sid,
                                  u8 fm, u64 type);
            void (*flush_iotlb)(struct intel_iommu *iommu, u16 did, u64 addr,
                                unsigned int size_order, u64 type, 
                                struct list_head *iovas,
                                struct list_head *to_free_iovas);
            void (*flush_process)(struct intel_iommu *iommu);
    };

    
# kernel启动的问题

## initrd

### 无法启动

### stuck in `Busybox`

#### `UUID does not exist`












----

