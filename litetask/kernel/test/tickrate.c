#include <stdio.h>
#include <time.h>
#include <dos.h>
#include <conio.h>
#include "intglue.h"

char far *screen = (char far *)0xB8000000;
unsigned short timerCnt=0, timerMax=65535;

char tickStack[1024];

/* XXXX - This is just to keep the linker happy */
void far * far schedule(void far *arg)
{
   return arg;
}

/* This is used by intglue.asm to set a INT vector */
void far * far setVector(int intr, void far *addr)
{
void far *oldVec;

/* Be nice - use DOS :) */
   oldVec = _dos_getvect(intr);
   _dos_setvect(intr, addr);
   return oldVec;
}

void far myTick(int irq)
{
static int ctr=0;
static char chs[] = "0123456789";

   if(timerCnt+timerMax+1 <= timerCnt)
      chainIRQ(irq);
   else
      outp(0x20, 0x20);
   timerCnt=timerCnt+timerMax+1;

   screen[0] = chs[ctr/100];
   screen[2] = chs[(ctr%100)/10];
   screen[4] = chs[ctr%10];
   ctr = (ctr+1)%1000;
   outp(0x61, inp(0x61) ^ 3);
}

void main(void)
{
time_t tv;
int i;
unsigned short temp;
char *str, buf[40];

   puts("Setting clock interrupt");
   outp(0x42, 0xFF);
   outp(0x42, 0xFF);
   if(setIRQTrap(0, myTick, tickStack+sizeof(tickStack)))
      return;
   for(;;)
   {
      printf("Enter timer maximum: ");
      while(!kbhit())
      {
         time(&tv);
         str = ctime(&tv);
         for(i=0; str[i] && str[i] != '\n'; i++)
            screen[10+2*i] = str[i];
      }
      gets(buf);
      if(sscanf(buf, "%u", &temp) == 1)
      {
         _disable();
         timerMax=temp;
         outp(0x43,0x34);
         outp(0x40, timerMax);
         outp(0x40, timerMax >> 8);
         _enable();
         printf("Set maximum=%u\n", timerMax);
      }
      else
      {
         _disable();
         timerMax=65535;
         outp(0x43,0x34);
         outp(0x40, 0xFF);
         outp(0x40, 0xFF);
         _enable();
         clearIRQTrap(0);
         outp(0x61, inp(0x61) & 0xFC);
         return;
      }
   }
}
