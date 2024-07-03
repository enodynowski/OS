#include "page.h"
#include <stddef.h>

struct ppage physical_page_array[128]; // 128 pages, each 4mb in length covers 256mbs of memory

struct ppage *head = NULL;

/* 
 * Initializes a linked list of free pages. Will loop over the physical_page_array and allocate each page and link it to the next and previous. 
 */
void init_pfa_list(void){
	//set a head that points at the first free page
	head = &physical_page_array[0];
	extern int _end_kernel;
	head->prev = NULL;
	void* starting_address = (((unsigned int)&_end_kernel)&0xffffff000) + 4096; // set the starting address for our pfa list by ensuring that it is after the memory area where our code lives

	// initialize the very first page of the loop
	physical_page_array[0].physical_addr = starting_address;
	physical_page_array[0].next = &physical_page_array[1];
	physical_page_array[0].prev = &head;

	
	// initialize all of the other pages 
	for (int i = 1; i < 127; i++){
		//for each element, set next to the next one, and prev to the previous one
		physical_page_array[i].physical_addr = physical_page_array[i-1].physical_addr+4096;
		physical_page_array[i].next = &physical_page_array[i+1];
		physical_page_array[i].prev = &physical_page_array[i-1];
	}
	
	// cleanup the last page to ensure that the last->next is NULL. 
	physical_page_array[127].physical_addr = physical_page_array[126].physical_addr+4096;
	physical_page_array[127].next=NULL;
	physical_page_array[127].prev = &physical_page_array[126];
}

/*
 * this is removing n items from the physical_page_array list such that they can be used for anything later on 
 */
struct ppage *allocate_physical_pages(unsigned int npages){
	// start an iterator at the head of the linked list, and a new linked of allocated 
	struct ppage* iterator = head;
	struct ppage* allocated_pages = head;
	
	for (int i =0; i < npages; i++){
	
		iterator = iterator->next;
	}
	
	iterator->prev->next=NULL;
	iterator->prev=NULL;
	head = iterator;
	
	return allocated_pages;
	
}

//this is adding items to linked list
void free_physical_pages(struct ppage *ppage_list){

	struct ppage* iterator = ppage_list;
	while (iterator->next != NULL){
		iterator = iterator->next;
	}
	iterator->next = head;
	head->prev = iterator;
	head = ppage_list;

}
