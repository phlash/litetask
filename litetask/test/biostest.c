#include "litetask.h"
#include "bios.h"

int mainStackSize = MINSTACK;
semaphore_t sigSem;

/* device handles */
int bd0=-1, bd1=-1, bd80=-1, bd81=-1;

char buffer[5120];

void far idleFun(void)
{
static int i = 0;
static char twiddle[] = "-\|/";

   textDrv.OutChXY(79,0,twiddle[i]);
   i = (i+1) % 4;
}

void far readTask(int device)
{
int rv;
char buf[40], far *bp = buffer;

/* check device is valid */
   if(device < 0)
   {
      sprintf(buf, "%i: Invalid device\r\n", device);
      printk(buf);
      putSemaphore(&sigSem);
      return;
   }

/* read from specified device */
   sprintf(buf, "%i starting.\r\n", device);
   printk(buf);
   if(openDevice(device, -1L))
   {
      sprintf(buf, "%i: cannot open.\r\n", device);
      printk(buf);
      putSemaphore(&sigSem);
      return;
   }
   switch(rv=readDevice(device, bp, 10L, 0L))
   {
   case 0:
      sprintf(buf, "%i done.\r\n", device);
      break;
   default:
      if(rv < 0)
         sprintf(buf, "%i cock-up %i.\r\n", device, rv);
      else
         sprintf(buf, "%i read %i.\r\n", device, rv);
   }
   closeDevice(device);
   printk(buf);
   putSemaphore(&sigSem);
}

void far mainTask(void)
{
int i;
char buf[80], far *bp = buffer;

   setPreEmptive(1);
   setIdleHook(idleFun);
//   installInt15();
   if(installBIOSDisk() < 0)
      return;
   bd0=createBIOSDisk(0);
   bd1=createBIOSDisk(1);
   bd80=createBIOSDisk(0x80);
   bd81=createBIOSDisk(0x81);
   sprintf(buf, "mainTask: buffer @ 0x%x:%x\r\n", FP_SEG(bp), FP_OFF(bp));
   printk(buf);
   printk("mainTask: delaying for 10 seconds\r\n");
   delayTask(180L);
   initSemaphore(&sigSem);
   getSemaphore(&sigSem, NOTIMEOUT);
   newTask(MINSTACK, readTask, sizeof(int), bd0);
   newTask(MINSTACK, readTask, sizeof(int), bd1);
   newTask(MINSTACK, readTask, sizeof(int), bd80);
   newTask(MINSTACK, readTask, sizeof(int), bd81);
   for(i=0; i<4; i++)
      getSemaphore(&sigSem, NOTIMEOUT);
   printk("mainTask: removing drivers..\r\n");
   removeBIOSDisk();
//   removeInt15();
}

