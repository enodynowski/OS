#include <fcntl.h>
#include <unistd.h>
#include "fat.h"
#include <stdio.h>


#define BYTES_PER_SECTOR 512

char buffer[512];
char rde_buff[512];

int read_sector(int fd, unsigned int sector_num, char *buf) {
  lseek(fd, (BYTES_PER_SECTOR * sector_num), SEEK_SET);
  read(fd, buffer, 512);
}

void main(){
  int fd = open("test.img", O_RDONLY);
  read_sector(fd, 0, buffer);
  struct boot_sector* bootSector = buffer;
  bootSector->bytes_per_sector;
  int sectorPerFAT   = bootSector->num_sectors_per_fat;
  int rde_start = bootSector->num_reserved_sectors + bootSector->num_sectors_per_fat + bootSector->num_fat_tables;
  read_sector(fd, rde_start, rde_buff);
  struct root_directory_entry* rde = rde_buff;




  printf("a) num_sectors_per_cluster = %d\n", bootSector->num_sectors_per_cluster);
  printf("b) num_reserved_sectors = %d\n", bootSector->num_reserved_sectors);
  printf("c) num_fat_tables = %d\n", bootSector->num_fat_tables);
  printf("d) num_root_dir_entries = %d\n", bootSector->num_root_dir_entries);


  printf("e) rde_one = %s\n", rde->file_name);
}
