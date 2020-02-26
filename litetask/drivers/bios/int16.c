#include <stdio.h>
#include <signal.h>
#include <dos.h>

extern void far int15inst(void);
extern void far int15dele(void);

static char far *screen = (char far *)0xB8000000;
static void (interrupt far *oldInt9)(void);

void hexDump(int hex, int where)
{
// Format byte into hex and put on screen at where
   if((hex & 0xF0) > 0x90)
      screen[where] = ((hex & 0xF0) >> 4) - 10 + '0';
   else
      screen[where] = ((hex & 0xF0) >> 4) + '0';
   if((hex & 0xF) > 9)
      screen[where+2] = (hex & 0xF) - 10 + '0';
   else
      screen[where+2] = (hex & 0xF) + '0';
}

void interrupt far int09trap(void)
{
static int cnt = 0;

// Display interrupt count and chain
   hexDump(++cnt, 0);
   _chain_intr(oldInt9);
}

void far _loadds scancode(int scan)
{
static int loc = 160;

// dump scancode on screen
   hexDump(scan, loc);
   loc += 6;
}

void main(void)
{
union REGS regs;

   puts("Press a key, <BS> to exit..");
   signal(SIGINT, SIG_IGN);
   oldInt9 = _dos_getvect(0x9);
   _dos_setvect(0x9, int09trap);
   int15inst();
   for(;;)
   {
      regs.x.ax = 0;
      int86(0x16, &regs, &regs);
      printf("%04X ", regs.x.ax);
      if(regs.h.al == 0x8)
         break;
      if(regs.h.al == 0x1b)
         system("command");
   }
   int15dele();
   _dos_setvect(0x9, oldInt9);
}
