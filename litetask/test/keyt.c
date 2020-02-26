#include <stdio.h>
#include <dos.h>

void main(void)
{
union REGS regs;

   while(regs.h.al != 8)
   {
      regs.h.ah = 0;
      int86(0x16, &regs, &regs);
      printf("%d(%d) ", regs.h.al, regs.h.ah);
   }
}
