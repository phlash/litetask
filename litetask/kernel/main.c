/*------------------------------------------------------------------------
   MAIN.C - LiteTask main function

   $Author:   Phlash  $
   $Date:   24 Jun 1995 20:24:20  $
   $Revision:   1.0  $

------------------------------------------------------------------------*/
#include "litetask.h"
#include "debug.h"
#include "kernel.h"

/* A label at the end of main()'s stack */
extern char far stackEnd;

/* The current version number */
static short version = LITETASK_VERSION;

/*
 * main() - Provides entry/exit point for kernel / user code etc.
 */
void far main(char far *commandTail)
{
union REGS regs;
unsigned short heapSeg;
int i, exitCondition, setCPU = 1;
char op, far *opts, far *opArg, far *pstackEnd, buf[40];

/* initialise heap */
   pstackEnd = &stackEnd;
   heapSeg = FP_SEG(pstackEnd) + (FP_OFF(pstackEnd) + 15)/16;
   int86(0x12, &regs);
   setHeapStart(heapSeg, regs.x.ax);

/* say hello */
   printk("LiteTask Multi-Tasking Kernel: copyright (AshbySoft *), 1992-1995\r\n");
   sprintf(buf, "Version: %i.%i\r\n", version >> 8, version & 0xFF);
   printk(buf);
   printk("Command line: ");
   printk(commandTail);
   printk("\r\n");

/* interpret command line options */
   opts = "hH?d:";
   while((op=getopts(commandTail, opts, &opArg)) > 0)
   {
      switch(op)
      {
      case 'h':
      case 'H':
      case '?':
         printk("LiteTask options: -h/? - Help, this text\r\n");
         printk("                  -d<arg> - Debug control\r\n");
         printk("                            use -dh for more\r\n");
         return;
      case 'd':
      case 'D':
         if(opArg)
         {
            for(i=0; opArg[i] && opArg[i] != ' '; i++)
            {
               switch(opArg[i])
               {
               case 'h':
               case 'H':
                  printk("Debug options: 0 - Turn off default debugging\r\n");
                  printk("               c - C library debugging\r\n");
                  printk("               i - I/O system debugging\r\n");
                  printk("               u - User level debugging\r\n");
                  printk("               e - enable Event output\r\n");
                  printk("               t - enable Tracing output\r\n");
                  printk("               n - no CPU traps (for Codeview)\r\n");
                  return;
               case '0':
                  debugFlags = 0;
                  break;
               case 'c':
               case 'C':
                  debugFlags |= DBG_SRC_CLIB;
                  break;
               case 'i':
               case 'I':
                  debugFlags |= DBG_SRC_IOSYS;
                  break;
               case 'u':
               case 'U':
                  debugFlags |= DBG_SRC_USR0;
                  break;
               case 'e':
               case 'E':
                  debugFlags |= DBG_TYPE_EVENT;
                  break;
               case 't':
               case 'T':
                  debugFlags |= DBG_TYPE_TRACE;
                  break;
               case 'n':
               case 'N':
                  setCPU = 0;
                  break;
               }
            }
         }
         else
         {
            printk("-d option requires an argument\r\n");
            return;
         }
         break;
      default:
         printk("Invalid return value from getopts()!\r\n");
         return;
      }
   }

/* reset options for mainTask */
   resetopts();

/* trap any CPU errors (eg: divide by zero) */
   if(setCPU)
      setCPUTraps();

/* trap any BIOS errors (eg: <CTRL>-<Break>) */
   setBIOSTraps();

/* initialise the timer manager */
   installTimer(65535);

/* start up the scheduler */
   exitCondition = startScheduler(commandTail);

/* dis-connect the timer */
   removeTimer();

/* restore traps */
   clearBIOSTraps();
   if(setCPU)
      clearCPUTraps();

/* Say why we died */
   sprintf(buf, "main(): termination code: %i\r\n", exitCondition);
   LT_DBG(DBG_KERN_ERROR, buf);
}

/*
 * Every kernel *must* have a function called 'panic' :-)
 */
void far panic(char far *msg)
{
   biosStr("Panic: ");
   biosStr(msg);
   for(;;)
   {
      biosStr("\r\nPress a key to reboot..");
      while(!biosKey(BK_PEEK));
      outp(0x64, 0xF0);
   }
}

/* End */
