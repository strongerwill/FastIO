1. 路径： arch\x86\include\asm\xen\hypercall.h
   相关行数：#467~495 
   sloc: 15   

2. 路径：include\xen\interface\xen.h
   相关行数：#64~67 278~301
   sloc: 21
   
3. 路径： arch\x86\include\asm\pgalloc.h
   相关行数：#106~208
   修改函数：pmd_alloc_one pmd_free()
   sloc: 19

4. 路径： arch\x86\mm\pgtable.c
   相关函数：alloc_pgd_page()，alloc_pmd_page()，alloc_pte_page()，free_pgd_page()，free_pmd_page()，free_pte_page() 
   修改函数：pgd_alloc()，pgd_free()，pte_alloc_one()，___pte_free_tlb()，___pmd_free_tlb()
   注：包括若干变量和头文件
   sloc: 10 + (93-66+1=28) + (120-95+1=26) + (147-123+1=25) + 52 + 51 + 49 + 18 + 18 + 18 
   total: 295


   共计：350

 
