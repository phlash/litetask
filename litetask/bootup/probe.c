/* Probe.c - Probe BIOS/Hardware for PC configuration */

#include <stdio.h>
#include <dos.h>

void biosList(unsigned short ax)
{
   printf("BIOS equipment list: 0x%X\n", ax);
   if(ax & 1)
      printf("- Floppy drive\n");
   else
      printf("- no floppy drive\n");
   if(ax & 2)
      printf("- math co-processor\n");
   else
      printf("- no math co-processor\n");
   printf("- system board RAM: %uk\n", (ax & 0xC) << 2);
   printf("- video mode: %d\n", (ax & 0x30) >> 4);
   printf("- floppy drives: %d\n", (ax & 0xC0) >> 6);
   if(ax & 0x100)
      printf("- DMA controller\n");
   else
      printf("- no DMA controller\n");
   printf("- serial ports: %d\n", (ax & 0xE00) >> 9);
   if(ax & 0x1000)
      printf("- game adapter (joystick port)\n");
   else
      printf("- no game adapter (joystick port)\n");
   if(ax & 0x2000)
      printf("- internal modem\n");
   else
      printf("- no internal modem\n");
   printf("- printers: %d\n", (ax & 0xC000) >> 14);
}

void biosDisk(int drive)
{
union REGS regs;
struct SREGS sregs;
unsigned char far *mpt;
int i;

/* Clear register info */
   memset((char *)&regs, 0, sizeof(regs));
   segread(&sregs);

/* Read BIOS disk information */
   regs.h.ah = 8;
   regs.h.dl = drive;
   int86x(0x13, &regs, &regs, &sregs);
   if(regs.x.cflag || !regs.h.dl)
      return;
   FP_SEG(mpt) = sregs.es;
   FP_OFF(mpt) = regs.x.di;
   printf("BIOS Disk 0x%X (of %d): %u/%u/%u MPT(@%p):", drive, regs.h.dl,
      ((unsigned short)regs.h.cl << 2) | regs.h.ch,
      regs.h.dh,
      regs.h.cl & 0x3F,
      mpt);
   for(i=0; i<11; i++)
      printf(" %02X", mpt[i]);
   putchar('\n');
}

void main(void)
{
union REGS regs;
int i;

/* Clear register info */
   memset((char *)&regs, 0, sizeof(regs));

/* Video state buffer (see \phil\silly\vid_bits\video.c) */

/* BIOS equipment list */
   int86(0x11, &regs, &regs);
   biosList(regs.x.ax);

/* BIOS memory size */
   int86(0x12, &regs, &regs);
   printf("BIOS memory size: %dk\n", regs.x.ax);

/* BIOS disk information */
   biosDisk(0);
   biosDisk(1);
   biosDisk(0x80);
   biosDisk(0x81);
}
