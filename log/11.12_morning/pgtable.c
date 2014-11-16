#include <linux/mm.h>
#include <linux/gfp.h>
#include <asm/pgalloc.h>
#include <asm/pgtable.h>
#include <asm/tlb.h>
#include <asm/fixmap.h>

//added by zhang
#include <asm/page.h>
#include <linux/slab.h>
#include <linux/ktime.h>
#include <asm/xen/hypercall.h>
#include <asm/xen/page.h>
#include <linux/module.h>


#define PGALLOC_GFP GFP_KERNEL | __GFP_NOTRACK | __GFP_REPEAT | __GFP_ZERO

#ifdef CONFIG_HIGHPTE
#define PGALLOC_USER_GFP __GFP_HIGHMEM
#else
#define PGALLOC_USER_GFP 0
#endif

gfp_t __userpte_alloc_gfp = PGALLOC_GFP | PGALLOC_USER_GFP;




struct ptrpte_t {
    struct page *content;
    //unsigned long content;
    struct ptrpte_t *next;
};



//added by zhang
DEFINE_SPINLOCK(pgd_cache_lock);
DEFINE_SPINLOCK(pmd_cache_lock); 
DEFINE_SPINLOCK(pte_cache_lock);
DEFINE_SPINLOCK(pgd_alloc_cnt_lock);
DEFINE_SPINLOCK(pmd_alloc_cnt_lock);
DEFINE_SPINLOCK(pte_alloc_cnt_lock);
DEFINE_SPINLOCK(pgd_free_cnt_lock);
DEFINE_SPINLOCK(pmd_free_cnt_lock);
DEFINE_SPINLOCK(pte_free_cnt_lock);

struct ptrpgd *pgd_head = NULL;
struct ptrpmd *pmd_head = NULL;
struct ptrpte_t *pte_head = NULL;
unsigned long  pgd_used_counter = 0;
unsigned long  pgd_free_counter = 0;
unsigned long  pmd_used_counter = 0;
unsigned long  pmd_free_counter = 0;
unsigned long  pte_used_counter = 0;
unsigned long  pte_free_counter = 0;


unsigned long alloc_pgd_page(void)
{
    unsigned long addr = 0;
    struct ptrpgd * tmpptr;
    if(pgd_head)
    {
        spin_lock(&pgd_cache_lock);
 	pgd_used_counter++;
        
	if(pgd_free_counter)
		pgd_free_counter--;
        
	addr = pgd_head->content;
        tmpptr = pgd_head;
        pgd_head = pgd_head->next;
        spin_unlock(&pgd_cache_lock);
        memset((void *)addr, 0, 4096);
        kfree(tmpptr);
        return addr;
    }
    else
    {   
        spin_lock(&pgd_cache_lock);
	pgd_used_counter++;
        spin_unlock(&pgd_cache_lock);
        return __get_free_page(PGALLOC_GFP);
    }
}

unsigned long alloc_pmd_page(void)
{
    unsigned long addr = 0;
    struct ptrpmd * tmpptr;
    if(pmd_head)
    {
        spin_lock(&pmd_cache_lock);
 	pmd_used_counter++;
	if(pmd_free_counter)
        	pmd_free_counter--;
        addr = pmd_head->content;
        tmpptr = pmd_head;
        pmd_head = pmd_head->next;
        spin_unlock(&pmd_cache_lock);
        memset((void *)addr, 0, 4096);
        kfree(tmpptr);
        return addr;
    }
    else
    {   
        spin_lock(&pmd_cache_lock);
	pmd_used_counter++;
        spin_unlock(&pmd_cache_lock);
        return __get_free_page(PGALLOC_GFP);
    }
}


struct page *alloc_pte_page(void)
{
    struct page *pte;
    struct ptrpte_t * tmpptr;
    if(pte_head)
    {
        spin_lock(&pte_cache_lock);
 	pte_used_counter++;
      	if(pte_free_counter) 
		pte_free_counter--;
        pte = pte_head->content;
        tmpptr = pte_head;
        pte_head = pte_head->next;
        spin_unlock(&pte_cache_lock);
        kfree(tmpptr);
        return pte;
    }
    else
    {   
        spin_lock(&pte_cache_lock);
	pte_used_counter++;
        spin_unlock(&pte_cache_lock);
        return alloc_pages(__userpte_alloc_gfp, 0);
    }
}
 
void free_pgd_page(unsigned long addr)
{
    struct ptrpgd *newstruct = NULL;
    struct ptrpgd *temp_head = NULL;
    int i = 0;
    int counter = 0;
    
    newstruct = (struct ptrpgd *)kmalloc(sizeof(struct ptrpgd), GFP_KERNEL);
    newstruct -> content = addr;

    spin_lock(&pgd_cache_lock);
    newstruct -> next = pgd_head;
    pgd_head = newstruct;
    temp_head = pgd_head;
    
    /*free node */
    if(pgd_used_counter)
    	pgd_used_counter--;
    pgd_free_counter++;
    //spin_unlock(&pgd_cache_lock);
   
    if(pgd_used_counter)
    { 
    	//if((pgd_free_counter/pgd_used_counter>=4) && ((pgd_used_counter + pgd_free_counter) >= 450))
    	//if((pgd_used_counter/pgd_free_counter < 4) && ((pgd_used_counter + pgd_free_counter) >= 150))
    	//if((pgd_used_counter/pgd_free_counter < 1) && (pgd_used_counter >= 12))
    	//if((pgd_free_counter/pgd_used_counter >= 6) && ((pgd_used_counter + pgd_free_counter) >= 50))
    	//if((pgd_used_counter/pgd_free_counter < 2) && ((pgd_used_counter + pgd_free_counter) >= 20))
    	if((pgd_free_counter/pgd_used_counter > 1) && ((pgd_used_counter + pgd_free_counter) >= 10))
    	//if((pgd_free_counter/pgd_used_counter >= 5) && ((pgd_used_counter + pgd_free_counter) >= 50))
    	{
        	counter = pgd_free_counter * 3 / 10;
        	//counter = 0;
        	for(i=0;i<counter;i++)
		{
	    		pgd_head = pgd_head->next;
		}
        	pgd_free_counter -= counter;
    	}
    }
    spin_unlock(&pgd_cache_lock);

    if(counter != 0)
    {
    	struct ptrpgd * newstructarray = NULL;
    	struct ptrpgd * newstructarray_head = NULL;
    	int rc = 1;
    	newstructarray = (struct ptrpgd *)kmalloc(sizeof(struct ptrpgd) * counter, GFP_KERNEL);
    	newstructarray_head = newstructarray;
        for (i=0;i<counter;i++)
        {
	    newstruct = temp_head;
	    temp_head = temp_head->next;
	    newstructarray[i].content = newstruct->content;
	    kfree(newstruct);
        }
	//hypercall newstructarray
	rc = HYPERVISOR_pgd_op(newstructarray, counter);
        //if (rc == 0)
 	//    printk("pgd cache free success\n");
	//else 
 	//    printk("pgd cache free error\n");
	    
	//free page to the buddy system
        newstructarray = newstructarray_head;
	for(i=0;i<counter;i++)
	{
	    free_page(newstructarray[i].content);
	}

	//free newstructarray
	kfree(newstructarray);
    }
    
    return;
}  

void free_pmd_page(unsigned long addr)
{
    struct ptrpmd *newstruct = NULL;
    struct ptrpmd *temp_head = NULL;
    int i = 0;
    int counter = 0;
    
    newstruct = (struct ptrpmd *)kmalloc(sizeof(struct ptrpmd), GFP_KERNEL);
    newstruct -> content = addr;

    spin_lock(&pmd_cache_lock);
    newstruct -> next = pmd_head;
    pmd_head = newstruct;
    temp_head = pmd_head;
    
    /*free node */
    if(pmd_used_counter)
    	pmd_used_counter--;
    pmd_free_counter++;
   
    if(pmd_used_counter) 
    {
	//if((pmd_free_counter/pmd_used_counter>=3) && ((pmd_used_counter + pmd_free_counter) >= 1800))
    	//if((pmd_used_counter/pmd_free_counter < 8) && ((pmd_used_counter + pmd_free_counter) >= 600))
    	//if((pmd_used_counter/pmd_free_counter < 1) && (pmd_used_counter >= 42))
    	//if((pmd_free_counter/pmd_used_counter >= 4) && (pmd_used_counter >= 80))
    	//if((pmd_free_counter/pmd_used_counter >= 6) && ((pgd_used_counter + pgd_free_counter) >= 230))
    	//if((pmd_used_counter/pmd_free_counter < 2) && ((pgd_used_counter + pgd_free_counter) >= 80))
    	if((pmd_free_counter/pmd_used_counter > 1) && ((pmd_used_counter + pmd_free_counter) >= 40))
    	//if((pmd_free_counter/pmd_used_counter >= 5) && ((pmd_used_counter + pmd_free_counter) >= 200))
    	{
        	counter = pmd_free_counter * 3 / 10;
        	//counter = 0;
       		for(i=0;i<counter;i++)
		{
	    		pmd_head = pmd_head->next;
		}
        	pmd_free_counter -= counter;
    	}
    }
    spin_unlock(&pmd_cache_lock);

    if(counter != 0)
    {
    	struct ptrpmd * newstructarray = NULL;
    	struct ptrpmd * newstructarray_head = NULL;
    	int rc = 1;
    	newstructarray = (struct ptrpmd *)kmalloc(sizeof(struct ptrpmd) * counter, GFP_KERNEL);
    	newstructarray_head = newstructarray;
        for (i=0;i<counter;i++)
        {
	    newstruct = temp_head;
	    temp_head = temp_head->next;
	    newstructarray[i].content = newstruct->content;
	    kfree(newstruct);
        }
	//hypercall newstructarray
	rc = HYPERVISOR_pmd_op(newstructarray, counter);
        //if (rc == 0)
 	    //printk("pmd cache free success\n");
	//else 
 	    //printk("pmd cache free error\n");
	    
	//free page to the buddy system
        newstructarray = newstructarray_head;
	for(i=0;i<counter;i++)
	{
	    free_page(newstructarray[i].content);
	}

	//free newstructarray
	kfree(newstructarray);
    }
    
    return;
}
 
/* 
void free_pmd_page(unsigned long addr)
{
    struct ptrpmd * newstruct = NULL;
    newstruct = (struct ptrpmd *)kmalloc(sizeof(struct ptrpmd), GFP_KERNEL);
    newstruct -> content = addr;
   

    spin_lock(&pmd_cache_lock);
    newstruct -> next = pmd_head;
    pmd_head = newstruct;
    

    pmd_used_counter--;
    pmd_free_counter++;
    spin_unlock(&pmd_cache_lock);
    return;
}
*/

//void free_pte_page(struct mmu_gather* tlb, struct page *pte)
void free_pte_page(struct page *pte)
{
    struct ptrpte_t *newstruct = NULL;
    struct ptrpte_t *temp_head = NULL;
    int i = 0;
    int counter = 0;
    
    newstruct = (struct ptrpte_t *)kmalloc(sizeof(struct ptrpte_t), GFP_KERNEL);
    newstruct -> content = pte;
    //newstruct -> mmu_tlb = tlb;

    spin_lock(&pte_cache_lock);
    newstruct -> next = pte_head;
    pte_head = newstruct;
    temp_head = pte_head;
    
    /*free node */
    if(pte_used_counter)
    	pte_used_counter--;
    pte_free_counter++;
    //spin_unlock(&pte_cache_lock);
   
    if(pte_used_counter)
    { 
    	//if((pte_free_counter/pte_used_counter>=8) && ((pte_used_counter + pte_free_counter) >= 2100))
    	//if(pte_used_counter + pte_free_counter >= 2100)
    	//if((pte_used_counter/pte_free_counter < 1) && (pte_used_counter >= 63))
    	if((pte_free_counter/pte_used_counter > 4) && ((pte_used_counter + pte_free_counter) >= 320))
    	//if((pte_free_counter/pte_used_counter >= 5) && ((pte_used_counter + pte_free_counter) >= 320))
    	{
 	        printk("pte free counter is %d\n", pte_free_counter);
 	        printk("pte used counter is %d\n", pte_used_counter);
        	counter = pte_free_counter * 3 / 10;
        	//counter = 0;
        	for(i=0;i<counter;i++)
		{
	    		pte_head = pte_head->next;
		}
        	pte_free_counter -= counter;
    	}
    }
    spin_unlock(&pte_cache_lock);

    if(counter != 0)
    {
    	struct ptrpte * newstructarray = NULL;
    	struct ptrpte * newstructarray_head = NULL;
    	int rc = 1;
    	newstructarray = (struct ptrpte *)kmalloc(sizeof(struct ptrpte) * counter, GFP_KERNEL);
    	newstructarray_head = newstructarray;
        for (i=0;i<counter;i++)
        {
	    newstruct = temp_head;
	    temp_head = temp_head->next;
	    //newstructarray[i].content = (unsigned long)page_address(newstruct->content);
	    newstructarray[i].content = pfn_to_mfn(page_to_pfn(newstruct->content));
	    //newstructarray[i].mmu_tlb = newstruct->mmu_tlb;
	    kfree(newstruct);
        }
	//hypercall newstructarray
	rc = HYPERVISOR_pte_op(newstructarray, counter);
        //if (rc == 0)
	//else 
 	    //printk("pte cache free error\n");
	    
	//free page to the buddy system
        newstructarray = newstructarray_head;
	for(i=0;i<counter;i++)
	{
	    //tlb_remove_page(newstructarray[i].mmu_tlb,newstructarray[i].content);
	    //__free_page(newstructarray[i].content);
	    free_page(newstructarray[i].content);
	}

	//free newstructarray
	kfree(newstructarray);
    }
   
    return;
} 

/* 
void free_pte_page(struct page *pte)
{
    struct ptrpte_t * newstruct = NULL;
    newstruct = (struct ptrpte_t *)kmalloc(sizeof(struct ptrpte_t), GFP_KERNEL);
    newstruct -> content = pte;
   

    spin_lock(&pte_cache_lock);
    newstruct -> next = pte_head;
    pte_head = newstruct;
  

    pte_used_counter--;
    pte_free_counter++;
    spin_unlock(&pte_cache_lock);
    return;
}
*/  
     
/*
void free_pte_page(unsigned long addr)
{
    struct ptrpte_t * newstruct = NULL;
    struct ptrpte_t * temp_head = NULL;
    //unsigned long  temp_counter = 0;
    //int rc = 1;
    newstruct = (struct ptrpte_t *)kmalloc(sizeof(struct ptrpte_t), GFP_KERNEL);
    newstruct -> content = pfn_to_mfn(PFN_DOWN(__pa((pte_t *)addr)));
    spin_lock(&pte_cache_lock);
    newstruct -> next = pte_head;
    pte_head = newstruct;
    //pte_free_page_counter++;
    temp_head = pte_head;
    //temp_counter = pte_free_page_counter;
    spin_unlock(&pte_cache_lock);

    printk("free pte page step0 \n");
    if(temp_counter<=2)
    {
    	rc = HYPERVISOR_pgtable_op(newstruct, temp_counter);
    	if (rc == 0)
 		printk("free pte page step1 \n");
    	else 
 		printk("free pte page step2 \n");
    }
    if((temp_counter >= 2000) && (pte_free_page_switch == 1))
    {
    	struct ptrpage * newstructarray = NULL;
    	struct ptrpage * newstructarray_head = NULL;
    	int i;
	newstructarray = (struct ptrpage *)kmalloc(sizeof(struct ptrpage) * temp_counter, GFP_KERNEL);
    	newstructarray_head = newstructarray;
        for (i=0;i<temp_counter;i++)
        {
	    newstruct = temp_head;
	    temp_head = temp_head->next;
	    newstructarray[i].content = newstruct->content;
	    kfree(newstruct);
        }	
	rc = HYPERVISOR_pgtable_op(newstructarray, temp_counter);
        if (rc == 0)
 	    printk("free pte page step3 \n");
	else 
 	    printk("free pte page step4 \n");
	
        //free page to the buddy system
        newstructarray = newstructarray_head;
	for(i=0;i<temp_counter;i++)
	{
	    free_page(newstructarray[i].content);
	}

 	printk("free pte page step5 \n");
	//free newstructarray
	kfree(newstructarray);
        pte_free_page_switch = 0;
    }
   return;
}
*/


pte_t *pte_alloc_one_kernel(struct mm_struct *mm, unsigned long address)
{
	return (pte_t *)__get_free_page(PGALLOC_GFP);
	//return (pte_t *)alloc_pte_page();
}

struct zz_pte {
    //struct page *content;
    unsigned long content;
    struct zz_pte *next;
};
DEFINE_SPINLOCK(zz_pte_lock);
struct zz_pte *zz_head = NULL;
void alloc_zz_pte(unsigned long addr)
{
    struct zz_pte * newstruct = NULL;
    struct zz_pte * temp = NULL;
    if(zz_head)
    {
	temp = zz_head;
    	while(temp != NULL)
	{
		if(addr != temp->content)
			temp = temp->next;
		else
			return;
    	}
    	newstruct = (struct zz_pte *)kmalloc(sizeof(struct zz_pte), GFP_KERNEL);
    	newstruct -> content = addr;
    	spin_lock(&zz_pte_lock);
    	newstruct -> next = zz_head;
    	zz_head = newstruct;
    	spin_unlock(&zz_pte_lock);
    }
    else
    {
    	newstruct = (struct zz_pte *)kmalloc(sizeof(struct zz_pte), GFP_KERNEL);
    	newstruct -> content = addr;
    	spin_lock(&zz_pte_lock);
    	newstruct -> next = zz_head;
    	zz_head = newstruct;
    	spin_unlock(&zz_pte_lock);
    } 
    return ;
}
int free_zz_pte(unsigned long addr)
{
    struct zz_pte * temp = NULL;
    struct zz_pte * bef = NULL;
    if(zz_head)
    {
	temp = zz_head;
	if(addr == temp->content)
	{
    		spin_lock(&zz_pte_lock);
		zz_head = temp->next;
        	kfree(temp);
    		spin_unlock(&zz_pte_lock);
		return 1;
	}
	else
	{
		bef = temp;
		temp = temp->next;
		while(temp != NULL)
		{
			if(addr != temp->content)
			{
				bef = temp;
				temp = temp->next;	
			}
			else
			{
    				spin_lock(&zz_pte_lock);
				bef->next = temp->next;
				kfree(temp);
    				spin_unlock(&zz_pte_lock);
				return 1;	
		
			}
		}
		return 0;
	}

    }	

    else 
	return 0;
}

static unsigned int pte_alloc_waste = 0;
static unsigned int pte_alloc_cnt = 0;

/* time of freeing page tables */
static unsigned int pgd_free_waste = 0;
static unsigned int pgd_free_cnt = 0;
unsigned int pmd_free_waste = 0;
unsigned int pmd_free_cnt = 0;
static unsigned int pte_free_waste = 0;
static unsigned int pte_free_cnt = 0;

/* turn on cache */
int cache_on = 0;
EXPORT_SYMBOL(cache_on);

/* turn on cache */
int timing_on = 0;
EXPORT_SYMBOL(timing_on);

pgtable_t pte_alloc_one(struct mm_struct *mm, unsigned long address)
{
	struct page *pte;
	//pte = alloc_pages(__userpte_alloc_gfp, 0);
	//added by zhang
	if(cache_on)
	{
	  if(timing_on)
	  {
		ktime_t local_tstart=ktime_get();	
        	pte = alloc_pte_page();
		ktime_t local_tend=ktime_get();
		s64 local_act_time=ktime_to_ns(ktime_sub(local_tend, local_tstart));
        
		spin_lock(&pte_alloc_cnt_lock);
		pte_alloc_waste += (unsigned int)local_act_time;
        	pte_alloc_cnt++;
		spin_unlock(&pte_alloc_cnt_lock);
	  }
	  else
        	pte = alloc_pte_page();
        }
	else
	{
	  if(timing_on)
	  {
		ktime_t local_tstart=ktime_get();	
		pte = alloc_pages(__userpte_alloc_gfp, 0);
		ktime_t local_tend=ktime_get();
		s64 local_act_time=ktime_to_ns(ktime_sub(local_tend, local_tstart));
        
		spin_lock(&pte_alloc_cnt_lock);
		pte_alloc_waste += (unsigned int)local_act_time;
        	pte_alloc_cnt++;
		spin_unlock(&pte_alloc_cnt_lock);

	  }
	  else
		pte = alloc_pages(__userpte_alloc_gfp, 0);
	}
	
	if (pte)
		pgtable_page_ctor(pte);
	
	//alloc_zz_pte((unsigned long)page_to_pfn(pte));
	return pte;
}

static int __init setup_userpte(char *arg)
{
	if (!arg)
		return -EINVAL;

	/*
	 * "userpte=nohigh" disables allocation of user pagetables in
	 * high memory.
	 */
	if (strcmp(arg, "nohigh") == 0)
		__userpte_alloc_gfp &= ~__GFP_HIGHMEM;
	else
		return -EINVAL;
	return 0;
}
early_param("userpte", setup_userpte);

void ___pte_free_tlb(struct mmu_gather *tlb, struct page *pte)
{
	pgtable_page_dtor(pte);
	paravirt_release_pte(page_to_pfn(pte));
	
	//tlb_remove_page(tlb, pte);
	
	//added by zhang
 	//free_pte_page((unsigned long)__va(PFN_PHYS(page_to_pfn(pte))));	
	//free_pte_page((unsigned long)page_address(pte));
 	if(cache_on)
	{
	  if(timing_on)
	  {	
		ktime_t local_tstart=ktime_get();	
		//free_pte_page(tlb, pte);
		free_pte_page(pte);
		ktime_t local_tend=ktime_get();
		s64 local_act_time=ktime_to_ns(ktime_sub(local_tend, local_tstart));
        
		spin_lock(&pte_free_cnt_lock);
		pte_free_waste += (unsigned int)local_act_time;
        	pte_free_cnt++;
		spin_unlock(&pte_free_cnt_lock);

	  }
	  else
		//free_pte_page(tlb,pte);
		free_pte_page(pte);
	}
	else
	{
	   if(timing_on)
	   {	
		ktime_t local_tstart=ktime_get();	
		tlb_remove_page(tlb, pte);
		ktime_t local_tend=ktime_get();
		s64 local_act_time=ktime_to_ns(ktime_sub(local_tend, local_tstart));
        
		spin_lock(&pte_free_cnt_lock);
		pte_free_waste += (unsigned int)local_act_time;
        	pte_free_cnt++;
		spin_unlock(&pte_free_cnt_lock);
	   }
	   else
		tlb_remove_page(tlb, pte);
	}
}

#if PAGETABLE_LEVELS > 2
void ___pmd_free_tlb(struct mmu_gather *tlb, pmd_t *pmd)
{
	paravirt_release_pmd(__pa(pmd) >> PAGE_SHIFT);
	//tlb_remove_page(tlb, virt_to_page(pmd));
        // added by zhang
	if(cache_on)
	{
	  if(timing_on)
	  {
		ktime_t local_tstart=ktime_get();	
		free_pmd_page((unsigned long)pmd);
		ktime_t local_tend=ktime_get();
		s64 local_act_time=ktime_to_ns(ktime_sub(local_tend, local_tstart));
        
		spin_lock(&pmd_free_cnt_lock);
		pmd_free_waste += (unsigned int)local_act_time;
        	pmd_free_cnt++;
		spin_unlock(&pmd_free_cnt_lock);
 	  }
	  else	
		free_pmd_page((unsigned long)pmd);
	}
	else
	{
	  if(timing_on)
	  {
		ktime_t local_tstart=ktime_get();	
		tlb_remove_page(tlb, virt_to_page(pmd));
		ktime_t local_tend=ktime_get();
		s64 local_act_time=ktime_to_ns(ktime_sub(local_tend, local_tstart));
        
		spin_lock(&pmd_free_cnt_lock);
		pmd_free_waste += (unsigned int)local_act_time;
        	pmd_free_cnt++;
		spin_unlock(&pmd_free_cnt_lock);
	  }
	  else
		tlb_remove_page(tlb, virt_to_page(pmd));
	}
}

#if PAGETABLE_LEVELS > 3
void ___pud_free_tlb(struct mmu_gather *tlb, pud_t *pud)
{
	paravirt_release_pud(__pa(pud) >> PAGE_SHIFT);
	tlb_remove_page(tlb, virt_to_page(pud));
}
#endif	/* PAGETABLE_LEVELS > 3 */
#endif	/* PAGETABLE_LEVELS > 2 */

static inline void pgd_list_add(pgd_t *pgd)
{
	struct page *page = virt_to_page(pgd);

	list_add(&page->lru, &pgd_list);
}

static inline void pgd_list_del(pgd_t *pgd)
{
	struct page *page = virt_to_page(pgd);

	list_del(&page->lru);
}

#define UNSHARED_PTRS_PER_PGD				\
	(SHARED_KERNEL_PMD ? KERNEL_PGD_BOUNDARY : PTRS_PER_PGD)


static void pgd_set_mm(pgd_t *pgd, struct mm_struct *mm)
{
	BUILD_BUG_ON(sizeof(virt_to_page(pgd)->index) < sizeof(mm));
	virt_to_page(pgd)->index = (pgoff_t)mm;
}

struct mm_struct *pgd_page_get_mm(struct page *page)
{
	return (struct mm_struct *)page->index;
}

static void pgd_ctor(struct mm_struct *mm, pgd_t *pgd)
{
	/* If the pgd points to a shared pagetable level (either the
	   ptes in non-PAE, or shared PMD in PAE), then just copy the
	   references from swapper_pg_dir. */
	if (PAGETABLE_LEVELS == 2 ||
	    (PAGETABLE_LEVELS == 3 && SHARED_KERNEL_PMD) ||
	    PAGETABLE_LEVELS == 4) {
		clone_pgd_range(pgd + KERNEL_PGD_BOUNDARY,
				swapper_pg_dir + KERNEL_PGD_BOUNDARY,
				KERNEL_PGD_PTRS);
	}

	/* list required to sync kernel mapping updates */
	if (!SHARED_KERNEL_PMD) {
		pgd_set_mm(pgd, mm);
		pgd_list_add(pgd);
	}
}

static void pgd_dtor(pgd_t *pgd)
{
	if (SHARED_KERNEL_PMD)
		return;

	spin_lock(&pgd_lock);
	pgd_list_del(pgd);
	spin_unlock(&pgd_lock);
}

/*
 * List of all pgd's needed for non-PAE so it can invalidate entries
 * in both cached and uncached pgd's; not needed for PAE since the
 * kernel pmd is shared. If PAE were not to share the pmd a similar
 * tactic would be needed. This is essentially codepath-based locking
 * against pageattr.c; it is the unique case in which a valid change
 * of kernel pagetables can't be lazily synchronized by vmalloc faults.
 * vmalloc faults work because attached pagetables are never freed.
 * -- wli
 */

#ifdef CONFIG_X86_PAE
/*
 * In PAE mode, we need to do a cr3 reload (=tlb flush) when
 * updating the top-level pagetable entries to guarantee the
 * processor notices the update.  Since this is expensive, and
 * all 4 top-level entries are used almost immediately in a
 * new process's life, we just pre-populate them here.
 *
 * Also, if we're in a paravirt environment where the kernel pmd is
 * not shared between pagetables (!SHARED_KERNEL_PMDS), we allocate
 * and initialize the kernel pmds here.
 */
#define PREALLOCATED_PMDS	UNSHARED_PTRS_PER_PGD

void pud_populate(struct mm_struct *mm, pud_t *pudp, pmd_t *pmd)
{
	paravirt_alloc_pmd(mm, __pa(pmd) >> PAGE_SHIFT);

	/* Note: almost everything apart from _PAGE_PRESENT is
	   reserved at the pmd (PDPT) level. */
	set_pud(pudp, __pud(__pa(pmd) | _PAGE_PRESENT));

	/*
	 * According to Intel App note "TLBs, Paging-Structure Caches,
	 * and Their Invalidation", April 2007, document 317080-001,
	 * section 8.1: in PAE mode we explicitly have to flush the
	 * TLB via cr3 if the top-level pgd is changed...
	 */
	flush_tlb_mm(mm);
}
#else  /* !CONFIG_X86_PAE */

/* No need to prepopulate any pagetable entries in non-PAE modes. */
#define PREALLOCATED_PMDS	0

#endif	/* CONFIG_X86_PAE */

static void free_pmds(pmd_t *pmds[])
{
	int i;

	for(i = 0; i < PREALLOCATED_PMDS; i++)
		if (pmds[i])
		{
			printk("free pmds\n");
			free_page((unsigned long)pmds[i]);
		}
}


static unsigned int pmd_alloc_cnt = 0;
static unsigned int pmd_alloc_waste = 0;
static int preallocate_pmds(pmd_t *pmds[])
{
	int i;
	bool failed = false;
	pmd_t *pmd;

	//printk("preallocate pmds 1000\n");
	for(i = 0; i < PREALLOCATED_PMDS; i++) {
		//pmd_t *pmd = (pmd_t *)__get_free_page(PGALLOC_GFP);
        	// added by zhang
		if(cache_on)
		{
		  if(timing_on)
		  {	
			ktime_t local_tstart=ktime_get();	
			pmd = (pmd_t *)alloc_pmd_page();
			ktime_t local_tend=ktime_get();
			s64 local_act_time=ktime_to_ns(ktime_sub(local_tend, local_tstart));
			
			spin_lock(&pmd_alloc_cnt_lock);
        		pmd_alloc_waste += (unsigned int)local_act_time;
        		pmd_alloc_cnt++;
			spin_unlock(&pmd_alloc_cnt_lock);
	          }
		  else	
			pmd = (pmd_t *)alloc_pmd_page();
		}
		else
		{
		  if(timing_on)
		  {
			ktime_t local_tstart=ktime_get();	
			pmd = (pmd_t *)__get_free_page(PGALLOC_GFP);
			ktime_t local_tend=ktime_get();
			s64 local_act_time=ktime_to_ns(ktime_sub(local_tend, local_tstart));
			
			spin_lock(&pmd_alloc_cnt_lock);
        		pmd_alloc_waste += (unsigned int)local_act_time;
        		pmd_alloc_cnt++;
			spin_unlock(&pmd_alloc_cnt_lock);
		  }
		  else
			pmd = (pmd_t *)__get_free_page(PGALLOC_GFP);
			
		}
	
		if (pmd == NULL)
			failed = true;
		pmds[i] = pmd;
	}

	if (failed) {
		free_pmds(pmds);
		return -ENOMEM;
	}

	return 0;
}

/*
 * Mop up any pmd pages which may still be attached to the pgd.
 * Normally they will be freed by munmap/exit_mmap, but any pmd we
 * preallocate which never got a corresponding vma will need to be
 * freed manually.
 */
static void pgd_mop_up_pmds(struct mm_struct *mm, pgd_t *pgdp)
{
	int i;

	for(i = 0; i < PREALLOCATED_PMDS; i++) {
		pgd_t pgd = pgdp[i];

		if (pgd_val(pgd) != 0) {
			pmd_t *pmd = (pmd_t *)pgd_page_vaddr(pgd);

			pgdp[i] = native_make_pgd(0);

			paravirt_release_pmd(pgd_val(pgd) >> PAGE_SHIFT);
			pmd_free(mm, pmd);
		}
	}
}

static void pgd_prepopulate_pmd(struct mm_struct *mm, pgd_t *pgd, pmd_t *pmds[])
{
	pud_t *pud;
	unsigned long addr;
	int i;

	if (PREALLOCATED_PMDS == 0) /* Work around gcc-3.4.x bug */
		return;

	pud = pud_offset(pgd, 0);

 	for (addr = i = 0; i < PREALLOCATED_PMDS;
	     i++, pud++, addr += PUD_SIZE) {
		pmd_t *pmd = pmds[i];

		if (i >= KERNEL_PGD_BOUNDARY)
			memcpy(pmd, (pmd_t *)pgd_page_vaddr(swapper_pg_dir[i]),
			       sizeof(pmd_t) * PTRS_PER_PMD);

		pud_populate(mm, pud, pmd);
	}
}
/* calculate screen output time */
static int time_switch=1;
static ktime_t tstart, tend;
static unsigned int pgd_alloc_waste = 0;
static unsigned int pgd_alloc_cnt = 0;
static int tm_incret = 1;
//unsigned long  pgop_cnt = 0;

pgd_t *pgd_alloc(struct mm_struct *mm)
{
	pgd_t *pgd;
	pmd_t *pmds[PREALLOCATED_PMDS];

	//pgd = (pgd_t *)__get_free_page(PGALLOC_GFP);
	// added by zhang


	s64 act_time, local_act_time;
	ktime_t local_tstart, local_tend;
	int rc=1;
        
	//if(pgop_cnt<60000)
	if(cache_on == 0)
	{	

	  if(timing_on)
	  {	
		if(time_switch)
		{
			tstart=ktime_get();	

			local_tstart=ktime_get();	
		        pgd = (pgd_t *)__get_free_page(PGALLOC_GFP);
			local_tend=ktime_get();
			local_act_time=ktime_to_ns(ktime_sub(local_tend, local_tstart));
        		
			spin_lock(&pgd_alloc_cnt_lock);
			pgd_alloc_waste += (unsigned int)local_act_time;
        		pgd_alloc_cnt++;
			spin_unlock(&pgd_alloc_cnt_lock);

			time_switch=0;	
		
			// hypercall: timing is on 
			rc = HYPERVISOR_pgd_op(pgd_head, 9999);
		}
		else
		{
		
			local_tstart=ktime_get();	
		        pgd = (pgd_t *)__get_free_page(PGALLOC_GFP);
			local_tend=ktime_get();
			local_act_time=ktime_to_ns(ktime_sub(local_tend, local_tstart));
    			
			spin_lock(&pgd_alloc_cnt_lock);
        		pgd_alloc_waste += (unsigned int)local_act_time;
        		pgd_alloc_cnt++;
			spin_unlock(&pgd_alloc_cnt_lock);
			
			tend=ktime_get();	
			act_time=ktime_to_ms(ktime_sub(tend, tstart));
			if((unsigned int)act_time>=tm_incret*60*1000)
			{
				spin_lock(&pgd_alloc_cnt_lock);
        			printk("PGD alloc:(average:%uns per %u)\n", (pgd_alloc_waste*100)/pgd_alloc_cnt, pgd_alloc_cnt);
				pgd_alloc_waste=0;
				pgd_alloc_cnt=0;
				spin_unlock(&pgd_alloc_cnt_lock);
        			
			
				spin_lock(&pgd_free_cnt_lock);
        			printk("PGD free:(average:%uns per %u)\n", (pgd_free_waste*100)/pgd_free_cnt, pgd_free_cnt);
				pgd_free_waste=0;
				pgd_free_cnt=0;
				spin_unlock(&pgd_free_cnt_lock);


				spin_lock(&pmd_alloc_cnt_lock);
				printk("PMD alloc:(average:%uns per %u)\n", (pmd_alloc_waste*100)/pmd_alloc_cnt, pmd_alloc_cnt);
				pmd_alloc_waste=0;
				pmd_alloc_cnt=0;
				spin_unlock(&pmd_alloc_cnt_lock);
        			
				
				spin_lock(&pmd_free_cnt_lock);
				printk("PMD free:(average:%uns per %u)\n", (pmd_free_waste*100)/pmd_free_cnt, pmd_free_cnt);
				pmd_free_waste=0;
				pmd_free_cnt=0;
				spin_unlock(&pmd_free_cnt_lock);

				spin_lock(&pte_alloc_cnt_lock);
				printk("PTE alloc:(average:%uns per %u)\n", (pte_alloc_waste*100)/pte_alloc_cnt, pte_alloc_cnt);
				pte_alloc_waste=0;
				pte_alloc_cnt=0;
				spin_unlock(&pte_alloc_cnt_lock);
			
				
				spin_lock(&pte_free_cnt_lock);
				printk("PTE free:(average:%uns per %u)\n", (pte_free_waste*100)/pte_free_cnt, pte_free_cnt);
				pte_free_waste=0;
				pte_free_cnt=0;
				spin_unlock(&pte_free_cnt_lock);
					
				tm_incret+=1;	
				
				printk("\n");	
        			
			}
		}
	   }	
	   else

		pgd = (pgd_t *)__get_free_page(PGALLOC_GFP);


        }
	else
	{
		// hypercall: cache is on 
		//if(cache_on == 0)
		//{
		//	cache_on = 1;
		//	rc = HYPERVISOR_pgd_op(pgd_head, 1);
			//if (rc == 0)
 	    		//	printk("pgd cache enabled\n");
			//else 
 	    		//	printk("pgd cache disabled\n");
		//}
		//pgd = (pgd_t *)alloc_pgd_page();
	
	   if(timing_on)
	   {	         	 
		if(time_switch)
		{
			tstart=ktime_get();	

			local_tstart=ktime_get();	
			pgd = (pgd_t *)alloc_pgd_page();
			local_tend=ktime_get();
			local_act_time=ktime_to_ns(ktime_sub(local_tend, local_tstart));
        		
			spin_lock(&pgd_alloc_cnt_lock);
			pgd_alloc_waste += (unsigned int)local_act_time;
        		pgd_alloc_cnt++;
			spin_unlock(&pgd_alloc_cnt_lock);

			time_switch=0;	
		
			// hypercall: cache and timing is on 
			rc = HYPERVISOR_pgd_op(pgd_head, 9998);
		}
		else
		{
			local_tstart=ktime_get();	
			pgd = (pgd_t *)alloc_pgd_page();
			local_tend=ktime_get();
			local_act_time=ktime_to_ns(ktime_sub(local_tend, local_tstart));
    			
			spin_lock(&pgd_alloc_cnt_lock);
        		pgd_alloc_waste += (unsigned int)local_act_time;
        		pgd_alloc_cnt++;
			spin_unlock(&pgd_alloc_cnt_lock);
			
			tend=ktime_get();	
			act_time=ktime_to_ms(ktime_sub(tend, tstart));
			if((unsigned int)act_time>=tm_incret*60*1000)
			{
				/*
			
				spin_lock(&pgd_alloc_cnt_lock);
        			printk("PGD alloc:(average:%uns per %u)\n", (pgd_alloc_waste*100)/pgd_alloc_cnt, pgd_alloc_cnt);
				pgd_alloc_waste=0;
				pgd_alloc_cnt=0;
				spin_unlock(&pgd_alloc_cnt_lock);
        			
			
				spin_lock(&pgd_free_cnt_lock);
        			printk("PGD free:(average:%uns per %u)\n", (pgd_free_waste*100)/pgd_free_cnt, pgd_free_cnt);
				pgd_free_waste=0;
				pgd_free_cnt=0;
				spin_unlock(&pgd_free_cnt_lock);


				spin_lock(&pmd_alloc_cnt_lock);
				printk("PMD alloc:(average:%uns per %u)\n", (pmd_alloc_waste*100)/pmd_alloc_cnt, pmd_alloc_cnt);
				pmd_alloc_waste=0;
				pmd_alloc_cnt=0;
				spin_unlock(&pmd_alloc_cnt_lock);
        			
				
				spin_lock(&pmd_free_cnt_lock);
				printk("PMD free:(average:%uns per %u)\n", (pmd_free_waste*100)/pmd_free_cnt, pmd_free_cnt);
				pmd_free_waste=0;
				pmd_free_cnt=0;
				spin_unlock(&pmd_free_cnt_lock);

				spin_lock(&pte_alloc_cnt_lock);
				printk("PTE alloc:(average:%uns per %u)\n", (pte_alloc_waste*100)/pte_alloc_cnt, pte_alloc_cnt);
				pte_alloc_waste=0;
				pte_alloc_cnt=0;
				spin_unlock(&pte_alloc_cnt_lock);
			
				
				spin_lock(&pte_free_cnt_lock);
				printk("PTE free:(average:%uns per %u)\n", (pte_free_waste*100)/pte_free_cnt, pte_free_cnt);
				pte_free_waste=0;
				pte_free_cnt=0;
				spin_unlock(&pte_free_cnt_lock);
					
				tm_incret+=1;	
				printk("\n");	

        		        
				spin_lock(&pgd_cache_lock);
        			printk("PGD cache page numbers:%ld\n", pgd_free_counter);	
        			printk("PGD table page numbers:%ld\n", pgd_used_counter);	
				spin_unlock(&pgd_cache_lock);
			
			
        			spin_lock(&pmd_cache_lock);
        			printk("PMD cache page numbers:%ld\n", pmd_free_counter);	
        			printk("PMD table page numbers:%ld\n", pmd_used_counter);	
				spin_unlock(&pmd_cache_lock);

        			spin_lock(&pte_cache_lock);
        			printk("PTE cache page numbers:%ld\n", pte_free_counter);	
        			printk("PTE table page numbers:%ld\n", pte_used_counter);	
				spin_unlock(&pte_cache_lock);

				printk("\n");	
				*/



				tm_incret+=1;	
					
			}
		}
	   }
	   else	
		pgd = (pgd_t *)alloc_pgd_page();
			
	}

	if (pgd == NULL)
		goto out;

	mm->pgd = pgd;

	if (preallocate_pmds(pmds) != 0)
		goto out_free_pgd;

	if (paravirt_pgd_alloc(mm) != 0)
		goto out_free_pmds;

	/*
	 * Make sure that pre-populating the pmds is atomic with
	 * respect to anything walking the pgd_list, so that they
	 * never see a partially populated pgd.
	 */
	spin_lock(&pgd_lock);

	pgd_ctor(mm, pgd);
	pgd_prepopulate_pmd(mm, pgd, pmds);

	spin_unlock(&pgd_lock);

	return pgd;

out_free_pmds:
	free_pmds(pmds);
out_free_pgd:
	//free_page((unsigned long)pgd);
        // added by zhang
        if(cache_on)
		free_pgd_page((unsigned long)pgd);
	else
		free_page((unsigned long)pgd); 
out:
	return NULL;
}

void pgd_free(struct mm_struct *mm, pgd_t *pgd)
{
        //printk("pgd free step000 \n");
	pgd_mop_up_pmds(mm, pgd);
	pgd_dtor(pgd);
	paravirt_pgd_free(mm, pgd);
        
	//free_page((unsigned long)pgd);
        // added by zhang
        if(cache_on)
	{
	  if(timing_on)
	  {
		ktime_t local_tstart=ktime_get();	
		free_pgd_page((unsigned long)pgd);
		ktime_t local_tend=ktime_get();
		s64 local_act_time=ktime_to_ns(ktime_sub(local_tend, local_tstart));
        
		spin_lock(&pgd_free_cnt_lock);
		pgd_free_waste += (unsigned int)local_act_time;
        	pgd_free_cnt++;
		spin_unlock(&pgd_free_cnt_lock);
	  }
	  else
		free_pgd_page((unsigned long)pgd);
	}
	else
	{
	   if(timing_on)
	   {
		ktime_t local_tstart=ktime_get();	
		free_page((unsigned long)pgd); 
		ktime_t local_tend=ktime_get();
		s64 local_act_time=ktime_to_ns(ktime_sub(local_tend, local_tstart));
        
		spin_lock(&pgd_free_cnt_lock);
		pgd_free_waste += (unsigned int)local_act_time;
        	pgd_free_cnt++;
		spin_unlock(&pgd_free_cnt_lock);
           }
	   else
		free_page((unsigned long)pgd); 
	}
}

int ptep_set_access_flags(struct vm_area_struct *vma,
			  unsigned long address, pte_t *ptep,
			  pte_t entry, int dirty)
{
	int changed = !pte_same(*ptep, entry);

	if (changed && dirty) {
		*ptep = entry;
		pte_update_defer(vma->vm_mm, address, ptep);
		flush_tlb_page(vma, address);
	}

	return changed;
}

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
int pmdp_set_access_flags(struct vm_area_struct *vma,
			  unsigned long address, pmd_t *pmdp,
			  pmd_t entry, int dirty)
{
	int changed = !pmd_same(*pmdp, entry);

	VM_BUG_ON(address & ~HPAGE_PMD_MASK);

	if (changed && dirty) {
		*pmdp = entry;
		pmd_update_defer(vma->vm_mm, address, pmdp);
		flush_tlb_range(vma, address, address + HPAGE_PMD_SIZE);
	}

	return changed;
}
#endif

int ptep_test_and_clear_young(struct vm_area_struct *vma,
			      unsigned long addr, pte_t *ptep)
{
	int ret = 0;

	if (pte_young(*ptep))
		ret = test_and_clear_bit(_PAGE_BIT_ACCESSED,
					 (unsigned long *) &ptep->pte);

	if (ret)
		pte_update(vma->vm_mm, addr, ptep);

	return ret;
}

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
int pmdp_test_and_clear_young(struct vm_area_struct *vma,
			      unsigned long addr, pmd_t *pmdp)
{
	int ret = 0;

	if (pmd_young(*pmdp))
		ret = test_and_clear_bit(_PAGE_BIT_ACCESSED,
					 (unsigned long *)pmdp);

	if (ret)
		pmd_update(vma->vm_mm, addr, pmdp);

	return ret;
}
#endif

int ptep_clear_flush_young(struct vm_area_struct *vma,
			   unsigned long address, pte_t *ptep)
{
	int young;

	young = ptep_test_and_clear_young(vma, address, ptep);
	if (young)
		flush_tlb_page(vma, address);

	return young;
}

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
int pmdp_clear_flush_young(struct vm_area_struct *vma,
			   unsigned long address, pmd_t *pmdp)
{
	int young;

	VM_BUG_ON(address & ~HPAGE_PMD_MASK);

	young = pmdp_test_and_clear_young(vma, address, pmdp);
	if (young)
		flush_tlb_range(vma, address, address + HPAGE_PMD_SIZE);

	return young;
}

void pmdp_splitting_flush(struct vm_area_struct *vma,
			  unsigned long address, pmd_t *pmdp)
{
	int set;
	VM_BUG_ON(address & ~HPAGE_PMD_MASK);
	set = !test_and_set_bit(_PAGE_BIT_SPLITTING,
				(unsigned long *)pmdp);
	if (set) {
		pmd_update(vma->vm_mm, address, pmdp);
		/* need tlb flush only to serialize against gup-fast */
		flush_tlb_range(vma, address, address + HPAGE_PMD_SIZE);
	}
}
#endif

/**
 * reserve_top_address - reserves a hole in the top of kernel address space
 * @reserve - size of hole to reserve
 *
 * Can be used to relocate the fixmap area and poke a hole in the top
 * of kernel address space to make room for a hypervisor.
 */
void __init reserve_top_address(unsigned long reserve)
{
#ifdef CONFIG_X86_32
	BUG_ON(fixmaps_set > 0);
	printk(KERN_INFO "Reserving virtual address space above 0x%08x\n",
	       (int)-reserve);
	__FIXADDR_TOP = -reserve - PAGE_SIZE;
#endif
}

int fixmaps_set;

void __native_set_fixmap(enum fixed_addresses idx, pte_t pte)
{
	unsigned long address = __fix_to_virt(idx);

	if (idx >= __end_of_fixed_addresses) {
		BUG();
		return;
	}
	set_pte_vaddr(address, pte);
	fixmaps_set++;
}

void native_set_fixmap(enum fixed_addresses idx, phys_addr_t phys,
		       pgprot_t flags)
{
	__native_set_fixmap(idx, pfn_pte(phys >> PAGE_SHIFT, flags));
}
