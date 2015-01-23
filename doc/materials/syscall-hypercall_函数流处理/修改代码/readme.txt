1. 文件hypercall.h 打桩位置可通过搜寻：dump_stack() 找到，只需使用dom0，重新编译Linux内核即可。
2. 锁的定义放在mmu.c的文件末尾处。
3. ./arch/x86/xen/mmu.c;/arch/x86/include/asm/xen/hypercall.h