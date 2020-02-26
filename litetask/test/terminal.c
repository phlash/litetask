#include "litetask.h"
#include "bios.h"
#include "serial.h"
#include "ioctl.h"

int mainStackSize = MINSTACK;

void far idleHook(void)
{
static int cnt = 0;
static char *twiddle = "-\|/";

   outchar(twiddle[cnt], 79, 0);
   cnt = (cnt+1) % 4;
}

void far mainTask(void)
{
int con, com2, baud, rv;
unsigned char b;

   setPreEmptive(1);
   setIdleHook(idleHook);
   
   con = installConsole();
   if(con < 0)
   {
      printk("Error installing BIOS console\r\n");
      return;
   }
   ioctlDevice(con, STDIOCNBLK, NULL);
   ioctlDevice(con, BCIOCNOECHO, NULL);

   writeDevice(con, "\f", 1, 0L);

   installSerial();
   com2 = createSerial(1, SER_MODE_8N1);
   if(com2 < 0)
   {
      printk("Oops, no COM2\r\n");
      removeConsole();
      return;
   }
   baud = 2400;
   ioctlDevice(com2, SERIOCSETBAUD, &baud);

/* Main program loop */
   writeDevice(con, "Terminal ready. Press CTRL+] to exit\n", 37, 0L);
   for(;;)
   {
      if((rv = readDevice(con, &b, 1, 0L)) < 0)
      {
         printk("Error reading console\r\n");
         break;
      }
      if(rv)
      {
         if(b == 29)       /* <Ctrl>+] */
            break;
         if(b == '\n')
         {
            b = '\r';
            while(!writeDevice(com2, &b, 1, 0L));
            b = '\n';
         }
         while(!writeDevice(com2, &b, 1, 0L));
      }
      if((rv = readDevice(com2, &b, 1, 0L)) < 0)
      {
         printk("Error reading COM2\r\n");
         break;
      }
      if(rv)
         writeDevice(con, &b, 1, 0L);
   }
   removeSerial();
   removeConsole();
}

