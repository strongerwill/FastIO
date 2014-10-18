1. 路径： arch\x86\include\asm\xen\hypercall.h
   相关行数：#467~495 
   
2. 路径：include\xen\interface\xen.h
   相关行数：#64~67 278~301
   
3. 路径： arch\x86\include\asm\pgalloc.h
   相关行数：#109~122
   修改函数：pmd_free()

4. 路径： arch\x86\mm\pgtable.c
   相关函数：alloc_pgd_page()，alloc_pmd_page()，alloc_pte_page()，free_pgd_page()，free_pmd_page()，free_pte_page() 
   修改函数：pgd_alloc()，pgd_free()，pte_alloc_one()，___pte_free_tlb()，___pmd_free_tlb()
   注：包括若干变量和头文件


 
