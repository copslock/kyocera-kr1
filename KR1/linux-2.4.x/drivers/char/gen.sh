rm -f tty_io.s
mips-linux-gcc -S  -D__KERNEL__ -I/home/mips/linux-2.4.19/include -Wall -Wstrict-prototypes -Wno-trigraphs -O1 -gdwarf-2 -fno-strict-aliasing -fno-common -DNO_MM -I /home/mips/linux-2.4.19/include/asm/gcc -G 0 -mno-abicalls -fno-pic -pipe -mcpu=r3000 -mips1   -nostdinc -I /opt/mips/lib/gcc-lib/mips-linux/3.2.3/include -DKBUILD_BASENAME=tty_io  -DEXPORT_SYMTAB -c tty_io.c 


