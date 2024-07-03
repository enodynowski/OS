#include <stdint.h>
#include "page.h"
#include "ide.h"
#include "fat.h"
#include "paging.h"

#define NULL 0

struct page pt[1024] __attribute__((aligned(4096)));
struct page pt2[1024] __attribute__((aligned(4096)));

//func to initialize all the entries in the page directory as not present, and set them all to rw
void init(struct page_directory_entry *pd){
    

	for (int i=0; i < 1023; i++){
    //init page directory
		pd[i].present=0;
		pd[i].rw = 0;
		pd[i].user = 0;
		pd[i].writethru = 0;
		pd[i].cachedisabled = 0;
		pd[i].accessed = 0;
		pd[i].pagesize = 0;
		pd[i].ignored = 0;
		pd[i].os_specific = 0;
		pd[i].frame = 0;

    //init page table
        pt[i].present = 1;
        pt[i].rw = 1;
        pt[i].user = 0;
        pt[i].accessed = 0;
        pt[i].dirty = 0;
        pt[i].unused = 0;
        pt[i].frame = i;

	}
    //add a page table at the first page directory index
    pd[0].present = 1;
    pd[0].rw = 1;
    //strip the trailing zeros by bitshifting to the right by 12 bits bc it's 4kb aligned
    pd[0].frame = (unsigned int) pt >> 12; 
	
    // Put the address of page_directory[] into CR3
	asm("mov %0,%%cr3"
	:
	: "r"(pd)
	:);
	asm("mov %cr0, %eax\n"
	"or $0x80000001,%eax\n"
	"mov %eax,%cr0");
}


void map_pages(void *vaddr, struct ppage *page, struct page_directory_entry *pd) { 

    // Calculate the page directory index and page table index from the virtual address
    uint32_t pd_index = ((uint32_t)vaddr >> 22) & 0x3FF;
    uint32_t pt_index = ((uint32_t)vaddr >> 12) & 0x3FF;

    // Check if the page directory entry is not present
    if (!pd[pd_index].present) {
        // Allocate a new page table and update the page directory entry
        pd[pd_index].frame = (uint32_t)page->physical_addr;
        pd[pd_index].present = 1;
    }

    // Update the page table entry with the physical address
    struct page *pt = (struct page *)(pd[pd_index].frame << 12);
    pt[pt_index].frame = (uint32_t)page->physical_addr;
    pt[pt_index].present = 1;
    pt[pt_index].rw = 1; // Set the entry to read/write

    return 0;
}
