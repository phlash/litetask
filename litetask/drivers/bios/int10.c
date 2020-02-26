#include <stdio.h>
#include <dos.h>

void main(void)
{
int i;
union REGS regs;

/* get video mode & current page */
   regs.x.ax = 0x0F00;
   int86(0x10, &regs, &regs);
   printf("Video mode: %d, Current video page: %d\n", regs.h.al, regs.h.bh);

/* write to each BIOS page */
   for(i=0; i<8; i++)
   {
      regs.h.ah = 0x09;
      regs.h.al = '0' + i;
      regs.h.bh = i;
      regs.h.bl = 0x07;
      regs.x.cx = 1;
      int86(0x10, &regs, &regs);
   }

/* cycle pages */
   for(i=0; i<8; i++)
   {
      regs.h.ah = 0x05;
      regs.h.al = i;
      int86(0x10, &regs, &regs);
      getch();
   }
   regs.h.ah = 0x05;
   regs.h.al = 0;
   int86(0x10, &regs, &regs);
}
