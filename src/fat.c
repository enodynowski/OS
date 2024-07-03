#include "fat.h"
#include "ide.h"
#include <stddef.h>
#define NULL 0


unsigned char bootSectorBuffer[512];
char rootDirectoryBuffer[8192];
char FATtableBuffer[16384];
struct boot_sector *bootSector;
struct root_directory_entry *rootDirectory;
int first_data_sector;

/**
 * C implementation for memory set. 
 * **/
void *memset(void *s, int c, size_t n){
    unsigned char *ptr = (unsigned char*)s;
    unsigned char v = (unsigned char)c;

    for (int i=0; i<n; i++){
        ptr[i] = v;
    }
    return s;
}

/**
 * C implementation for memory set.
 * **/
void memcpy(void * __restrict dest, const void * __restrict src, size_t num) {
    
    unsigned char *d = (unsigned char*)dest;
    unsigned char *s = (unsigned char*)src;

    for (int k=0; k<num; k++){
        d[k] = s[k];
    }
    return dest;
}



/**
 * Compare two string.
 * Reference: C library.
 * **/
int strcmp(char *s1, char *s2){
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    } 
    return *(const unsigned char*)s1 - *(const unsigned char*)s2; 
}

void removeSpace(char *str) {
    if (str == NULL) {
        return;
    }
    int n = 0;
    for (int i=0; i<sizeof(str); ++i) {
        if (str[i] != ' ') {
            str[n++] = str[i];
        }
    }
    str[n] = '\0';
}

/**
 * Compute the first sector of the cluster.
 * **/
int get_the_first_sector_of_cluster(int cluster) {
    return ((cluster - 2) * bootSector->num_sectors_per_cluster) + first_data_sector;
}




int fatInit() {
  bootSector = bootSectorBuffer;
  rootDirectory = rootDirectoryBuffer;

  ata_lba_read(2048, bootSector, 1);  // read boot sector

  // Compute the logical block adress of root directory entry.
  int rootDirectoryLba =
      2048 + bootSector->num_reserved_sectors +
      (bootSector->num_sectors_per_fat * bootSector->num_fat_tables);

  // Read the root directory in.
  ata_lba_read(rootDirectoryLba, rootDirectoryBuffer, 1);

  // Compute the useful information.

  int root_dir_sectors =
      (bootSector->num_root_dir_entries * 32) / bootSector->bytes_per_sector;
  first_data_sector =
      bootSector->num_reserved_sectors +
      ((bootSector->num_fat_tables * bootSector->num_sectors_per_fat) +
       root_dir_sectors) +
      2048;

  // Read FAT
  ata_lba_read(2048 + bootSector->num_reserved_sectors, FATtableBuffer, 32);
}

void fatRead(struct file *fd, unsigned char *buf){
    // Calcuate the first sector of the cluster.
    int cluster = fd->rde.cluster;
    int sector = get_the_first_sector_of_cluster(cluster);
    ata_lba_read(sector, buf, 4);
    return 0;
}

void fatOpen(char *path, struct file *fd){
    char fileNamePath[8];
    char fileNameRootDirectory[8];

    memset(fileNamePath, 0, sizeof(fileNamePath)); //zero out the filename path
    memcpy(fileNamePath, path, 8);


    //search for the file at the root level
    for (int i = 0; i < 10; i++)
    {
        memset(fileNameRootDirectory, 0, sizeof(fileNameRootDirectory));
        memcpy(fileNameRootDirectory, rootDirectory[i].file_name, 3);


        //remove the whitespace from the string
        removeSpace(fileNameRootDirectory);

        if (strcmp(fileNameRootDirectory, fileNamePath) == 0) {
            fd->rde = rootDirectory[i];
            fd->start_cluster = rootDirectory[i].cluster;
            break;
        }
    }
    return 0;
    
}