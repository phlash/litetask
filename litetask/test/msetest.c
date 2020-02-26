#include "litetask.h"
#include "bios.h"
#include "serial.h"
#include "mouse.h"
#include "ioctl.h"

int mainStackSize = MINSTACK;

void far idleHook(void)
{
static int cnt = 0;
static char *twiddle = "-\|/";

   textDrv.OutChXY(79, 0, twiddle[cnt]);
   cnt = (cnt+1) % 4;
}

void far mainTask(void)
{
int con, com1, mse, var1, var2, rv;
mousePos_t msePos;
char key, buf[40];

   setPreEmptive(1);
   setIdleHook(idleHook);
   installConsole();
   con = createConsole(0);
   ioctlDevice(con, BCIOCNOECHO, NULL);
   ioctlDevice(con, STDIOCNBLK, NULL);
   installSerial();
   installMouse();
   
   com1 = createSerial(0, SER_MODE_8N1);
   if(com1 < 0)
   {
      printk("Oops, no COM1\r\n");
      goto Error;
   }
   var1 = 1200;
   if(ioctlDevice(com1, SERIOCSETBAUD, &var1) < 0)
   {
      printk("Oops, can't set baud rate\r\n");
      goto Error;
   }
   var1 = MSE_MICROSOFT;
   var2 = MSE_DELTA_MODE;
   mse = createMouse(com1, var1);
   if(mse < 0)
   {
      printk("Oops, no mouse\r\n");
      goto Error;
   }
   ioctlDevice(mse, MSEIOCSETMODE, &var2);
   printk("Reading mouse messages, press:\r\n"); 
   printk("'m' to toggle Type, 'a' to toggle mode, <ESC> to quit..\r\n");
   for(;;)
   {
      if((rv=select(SEL_READ, 2, con, mse))<0)
      {
         printk("Oops, can't select on console / mouse\r\n");
         break;
      }
      if(rv == con)
      {
         key = 0;
         readDevice(con, &key, 1, 0L);
         if(key == 27)
            break;
         if(key == 'm')
         {
            if(var1 == MSE_MOUSESYSTEMS)
               var1 = MSE_MICROSOFT;
            else
               var1 = MSE_MOUSESYSTEMS;
            ioctlDevice(mse, MSEIOCSETTYPE, &var1);
         }
         if(key == 'a')
         {
            if(var2 == MSE_ABSOLUTE_MODE)
               var2 = MSE_DELTA_MODE;
            else
               var2 = MSE_ABSOLUTE_MODE;
            ioctlDevice(mse, MSEIOCSETMODE, &var2);
         }
      }
      if(rv == mse)
      {
         rv = readDevice(mse, &msePos, sizeof(mousePos_t), 0L); 
         if( rv != sizeof(mousePos_t) )
         {
            sprintf(buf, "Oops, can't read mouse: %i\r\n", rv);
            printk(buf);
            break;
         }
         if(var1 == MSE_MICROSOFT)
            printk("Microsoft/");
         else
            printk("M Systems/");
         if(var2 == MSE_DELTA_MODE)
            printk("Del ");
         else
            printk("Abs ");

         if(msePos.buttons & MSE_BUTTON_L)
            printk("L");
         else
            printk(" ");
         if(msePos.buttons & MSE_BUTTON_M)
            printk("M");
         else
            printk(" ");
         if(msePos.buttons & MSE_BUTTON_R)
            printk("R ");
         else
            printk("  ");
         sprintf(buf, "x=%i y=%i     \r", msePos.x, msePos.y);
         printk(buf);
      }
   }

Error:
   removeMouse();
   removeSerial();
   removeConsole();
}

