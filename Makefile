

all: hello rootfs.img
	mcopy -i rootfs.img@@1M hello ::/
	mmd -i rootfs.img@@1M boot
	mcopy -i rootfs.img@@1M grub.cfg ::/boot
	mcopy -i rootfs.img@@1M src/ide.s ::/
	
	
hello: src/hello.c src/rprintf.c
	gcc -c -ffreestanding -mgeneral-regs-only -mno-mmx -m32 -march=i386 -fno-pie -fno-stack-protector -g3 -Wall -o obj/hello.o src/hello.c
	gcc -c -ffreestanding -mgeneral-regs-only -mno-mmx -m32 -march=i386 -fno-pie -fno-stack-protector -g3 -Wall -o obj/rprintf.o src/rprintf.c
	gcc -c -ffreestanding -mgeneral-regs-only -mno-mmx -m32 -march=i386 -fno-pie -fno-stack-protector -g3 -Wall -o obj/page.o src/page.c
	gcc -c -ffreestanding -mgeneral-regs-only -mno-mmx -m32 -march=i386 -fno-pie -fno-stack-protector -g3 -Wall -o obj/paging.o src/paging.c		
	gcc -c -ffreestanding -mgeneral-regs-only -mno-mmx -m32 -march=i386 -fno-pie -fno-stack-protector -g3 -Wall -o obj/fat.o src/fat.c

	
	nasm -f elf32 -g -o obj/ide.o src/ide.s
	ld -T kernel.ld -e main -melf_i386 obj/rprintf.o obj/hello.o obj/page.o obj/fat.o obj/ide.o obj/paging.o -o hello


rootfs.img:
	dd if=/dev/zero of=rootfs.img bs=1M count=32
#	/usr/local/grub-i386/bin/grub-mkimage -p "(hd0,msdos1)/boot" -o grub.img -O i386-pc normal biosdisk multiboot multiboot2 configfile fat exfat part_msdos
	grub-mkimage -p "(hd0,msdos1)/boot" -o grub.img -O i386-pc normal biosdisk multiboot multiboot2 configfile fat exfat part_msdos
#	dd if=/usr/local/grub-i386/lib/grub/i386-pc/boot.img  of=rootfs.img conv=notrunc
	dd if=/usr/lib/grub/i386-pc/boot.img  of=rootfs.img conv=notrunc
	dd if=grub.img of=rootfs.img conv=notrunc seek=1
	echo 'start=2048, type=83, bootable' | sfdisk rootfs.img
	mkfs.vfat --offset 2048 -F16 rootfs.img

debug:
	screen -S qemu -d -m qemu-system-i386 -S -s -hda rootfs.img -monitor stdio
	TERM=xterm gdb -x gdb_os.txt && killall qemu-system-i386

clean:
	rm -f grub.img hello obj/*.o rootfs.img 
