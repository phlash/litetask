#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include "intglue.h"

char far *screen = (char far *)0xB8000000;
int loc=80;

char irqStack[16][1024];

/* XXXX - This is just to keep the linker happy */
void far * far schedule(int irq, void far *arg)
{
   return arg;
}

volatile int overrun=0;

void reboot(void)
{
long l;

   _disable();
   for(l=0L; l<1000000L; l++) screen[2*loc] = "-\|/"[(int)l&3];
   for(;;)
   {
      outp(0x64, 0xF0);
      for(l=0L; l<10000L; l++);
   }
}

void far panic(char far *msg, int irq)
{
int i;

   if(irq > 7)
      outp(0xA0, 0x20);
   if(irq >= 0)
      outp(0x20, 0x20);
   for(i=0; msg[i]; i++, loc++)
      screen[2*loc] = msg[i];
   if(irq >= 0)
      screen[2*loc] = (irq < 10) ? (char)irq + '0' : (char)(irq-10) + 'A';
   loc = (loc+79)/80*80;
   overrun=0;
   if(irq == -2)
      reboot();
}

void far * far setVector(int intr, void far *addr)
{
void far *oldVec;

/* Be nice - use DOS :) */
   oldVec = _dos_getvect(intr);
   _dos_setvect(intr, addr);
   return oldVec;
}

int far intHook(int irq)
{
static int irqSet[16];
int segdiff, offdiff;
char c, far *p1, far *p2;

/* Check irq number */
   if(irq<0 || irq>15)
   {
      screen[158] = '!';
      return;
   }
   c = (irq < 10) ? (char)irq + '0' : (char)(irq-10) + 'A';

/* Check we are on the correct stack */
   p1 = (char far *)irqStack[irq];
   p2 = (char far *)&irq;
   segdiff = FP_SEG(p2) - FP_SEG(p1);
   offdiff = FP_OFF(p2) - FP_OFF(p1);
   if(segdiff || offdiff < 0 || offdiff > 1024)
      screen[2*irq+80] = c;

/* put the IRQ number on screen */
   screen[2*irq] = (irqSet[irq]) ? ' ' : c;
   irqSet[irq] = 1-irqSet[irq];

/* call previous owner of interrupt */
   chainIRQ(irq);

/* cause an overrun if required */
   if(irq+1 == overrun)
      while(overrun);
   return 0;
}

int testIRQ0(void)
{
   overrun=1;
   while(overrun);
}

int testIRQ8(void)
{
union REGS regs;
struct SREGS sregs;
volatile char c, far *pc = &c;

   regs.x.ax = 0x8300;
   regs.x.cx = 15;
   regs.x.dx = 0;
   regs.x.bx = FP_OFF(pc);
   sregs.es = FP_SEG(pc);
   int86x(0x15, &regs, &regs, &sregs);
   overrun=9;
   while(overrun);
   regs.x.ax = 0x8301;
   int86(0x15, &regs, &regs);
}

void main(void)
{
int irq;

/* Hook IRQ0-15 */
   for(irq=0; irq<16; irq++)
   {
      if(setIRQTrap(irq, intHook, irqStack[irq]+1024-2))
      {
         printf("Oops, can't trap IRQ%d\n", irq);
         goto bailout;
      }
   }
   if(!setIRQTrap(irq=16, intHook, NULL))
   {
      puts("Oops, can trap IRQ16!");
      goto bailout;
   }
   if(!clearIRQTrap(16))
   {
      puts("Oops can clear IRQ16!");
      goto bailout;
   }
   if(!setIRQTrap(-1, intHook, NULL))
   {
      puts("Oops, can trap IRQ-1!");
      goto bailout;
   }
   if(!clearIRQTrap(-1))
   {
      puts("Oops, can clear IRQ-1!");
      goto bailout;
   }
   panic("Hello world!", -1);
   for(;;)
   {
      puts("Press '0' to test IRQ0 overrun");
      puts("      '8' to test IRQ8 overrun");
      puts("      's' to start a shell");
      puts("      'd' to test reboot");
      puts("      'q' to quit");
      switch(getch())
      {
      case '0':
         testIRQ0();
         break;
      case '8':
         testIRQ8();
         break;
      case 's':
      case 'S':
         puts("Type exit to terminate shell");
         system("command");
         break;
      case 'd':
         panic("****** About to die! *******", -2);
         break;
      case 'q':
      case 'Q':
         goto done;
      }
   }
done:
   for(irq=15; irq>=0; irq--)
      clearIRQTrap(irq);
   puts("Done!");
   exit(0);

bailout:
   while(irq--)
      clearIRQTrap(irq);
   exit(1);
}

