#include <stdio.h>
#include <dos.h>

static volatile int trapped;

void interrupt far handler(void)
{
union REGS regs;
   regs.h.ah = 0x0A;
   regs.h.al = 'C';
   regs.h.bh = 0;
   regs.h.bl = 7;
   regs.x.cx = 1;
   int86(0x10, &regs, &regs);
   trapped++;
}

void main(void)
{
void far *oldvec;

   oldvec = _dos_getvect(0x1b);
   _dos_setvect(0x1b, handler);
   puts("Press ctrl-break");
   while(!trapped);
   _dos_setvect(0x1b, oldvec);
}

