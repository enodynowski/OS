#include <stdio.h>
#include <stdint.h>
#include "page.h"
#include "ide.h"
#include "fat.h"
#include "paging.h"

#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6
#define KERNEL_START_VADDR              0x100000
struct page_directory_entry pd[1024] __attribute__((aligned(4096)));

unsigned int multiboot_header[] __attribute__((section(".multiboot"))) = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};



// Code that prints a character to the screen and implements scrolling in the terminal
int chars_on_scrn = 0;
void newputc (int c) {
	unsigned short *vram = (unsigned short*)0xb8000; // Base address of video mem
	const unsigned char color = 7;
	unsigned short data = (color << 8) + c;
	vram[chars_on_scrn] = data;
	chars_on_scrn ++;
	if (chars_on_scrn == 2000){
		for(int i = 0; i < 2000; i ++){
			vram[i] = vram[i+80];
		}
		chars_on_scrn = 1920;
	}
	
}

//Reads from I/O port and returns a 1 byte value
uint8_t inb (uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}
// Writes a 1 byte value to an I/O port
void outb (uint16_t _port, uint8_t val){
	__asm__ __volatile__ ("outb %0, %1" : : "a" (val), "dN" (_port));
}

unsigned char keyboard_map[128] =
{
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 9 */
	'9', '0', '-', '=', '\b',     /* Backspace */
	'\t',                 /* Tab */
	'q', 'w', 'e', 'r',   /* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
	0,                  /* 29   - Control */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',     /* 39 */
'\'', '`',   0,                /* Left shift */
'\\', 'z', 'x', 'c', 'v', 'b', 'n',                    /* 49 */
	'm', ',', '.', '/',   0,                              /* Right shift */
	'*',
	0,  /* Alt */
	' ',  /* Space bar */
	0,  /* Caps lock */
	0,  /* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0,  
	0,  /* < ... F10 */
	0,  /* 69 - Num lock*/
	0,  /* Scroll Lock */
	0,  /* Home key */
	0,  /* Up Arrow */
	0,  /* Page Up */
	'-',
	0,  /* Left Arrow */
	0,  
	0,  /* Right Arrow */
	'+',
	0,  /* 79 - End key*/
	0,  /* Down Arrow */
	0,  /* Page Down */
	0,  /* Insert Key */
	0,  /* Delete Key */
	0,   0,   0,  
	0,  /* F11 Key */
	0,  /* F12 Key */
	0,  /* All other keys are undefined */
};



void main() {

	//initialize the page directory, page table, and identity map all of the pages that make up the kernel
	init(pd);

	//initialize the Page frame allocator 
	init_pfa_list();
	//demonstrate that allocating physical pages works
	struct ppage* page = allocate_physical_pages(4);

	void *vaddr = KERNEL_START_VADDR;
	map_pages(vaddr, page, pd);

	free_physical_pages(page);
	
	fatInit();
	struct file *fd = 0x300000;
	fatOpen("FUN", fd);


	// Read the file.
    // Initialize a char array as file buffer.
    int fileSize = get_the_first_sector_of_cluster(fd->rde.file_size);
	esp_printf(newputc, "%d", fileSize);
	char fileBuffer[fileSize];
    memset(fileBuffer, 0, sizeof(fileBuffer));

    // Read the file.
    fatRead(fd, fileBuffer, fileSize);

    // Print out the data in the buffer. 
    esp_printf(newputc, "data read in: %s", fileBuffer);



  // reading from the PS/2 status register (0x64). Checking if the LSB is 1, and if it is, get the scancode and print
    while(1) {
    	
	uint8_t status = inb(0x64);

	
        if(status & 1) {
        	uint8_t scancode = inb(0x60);
        	//check if the scan code is a legitimate character or a break code, only print if its the character and not a break code
        	if (scancode < 128){
	        	newputc(keyboard_map[scancode]);

        	}
        }
    }
}
