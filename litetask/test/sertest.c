#include "litetask.h"
#include "ioctl.h"
#include "select.h"
#include "serial.h"

int mainStackSize = MINSTACK;

void far idleHook(void)
{
static int cnt = 0;
static char *twiddle = "-\|/";

   textDrv.OutChXY(79,0,twiddle[cnt]);
   cnt = (cnt+1) % 4;
}

void far subTask(void)
{
char *string = "AT\r";
int i, com2, baud = 2400;

/* Set to 2400 baud */
   com2 = createSerial(1, SER_MODE_8N1);
   if(com2 < 0)
   {
      printk("Oops, no COM2\r\n");
      return;
   }
   ioctlDevice(com2, SERIOCSETBAUD, &baud);
   ioctlDevice(com2, STDIOCBLK, NULL);
   printk("Writing to COM2\r\n");

/* write bytes to com2 all the time */
   for(i=0; i<50; i++)
   {
      writeDevice(com2, string, 3L, 0L);
      textDrv.OutStrXY(60,1,string);
      delayTask(1L);
      textDrv.OutStrXY(60,1,"   ");
   }
   deleteSerial(1);
}

void far mainTask(void)
{
int handle, baud, rv, i;
unsigned char b;
char buf[40];

   setPreEmptive(1);
   setIdleHook(idleHook);
   textDrv.Init();
   installSerial();
   newTask(MINSTACK, subTask, 0, NULL);
   delayTask(9L);
   handle = createSerial(0, SER_MODE_8N1);
   if(handle < 0)
   {
      printk("Oops, no COM1\r\n");
      return;
   }
   baud = 1200;
   ioctlDevice(handle, SERIOCSETBAUD, &baud);
   printk("Reading 96 bytes from serial port\r\n");
   for(i=0; i<96; i++)
   {
   again:
      if((rv=select(SEL_READ, 1, handle)) < 0)
      {
         sprintf(buf, "Error selecting on port %i\r\n", rv);
         printk(buf);
         break;
      }
      if((rv=readDevice(handle, &b, 1L, 0L)) < 0)
      {
         sprintf(buf, "Error reading port %i\r\n", rv);
         printk(buf);
         break;
      }
      if(rv == 0)
      {
         printk("Oops, nothing to read\r\n");
         goto again;
      }
      sprintf(buf, "%x ", b);
      printk(buf);
      if((i % 16) == 15)
         printk("\r\n");
   }
   deleteSerial(0);
}

