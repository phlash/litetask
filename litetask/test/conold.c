#include "litetask.h"
#include "bios.h"

int mainStackSize = MINSTACK;

static int consoleHandle;

void far appTask(int instance)
{
char ch, buf[80];

/* enter a loop, checking the console */
   for(;;)
   {
      if(openDevice(consoleHandle, 18L))
      {
         sprintf(buf, "App %i: Cannot open console\r\n", instance);
         printk(buf);
      }
      else
      {
         writeDevice(consoleHandle, buf, sprintf(buf, "Char %i:", instance), 0L);
         readDevice(consoleHandle, &ch, 1L, 0L);
         closeDevice(consoleHandle);
         delayTask(18L);
      }
   }
}

void far idleHook(void)
{
static char far *twiddles = "-\|/";
static int i = 0;

   outchar(twiddles[i], 79, 0);
   i = (i+1) % 4;
}

void far mainTask(void)
{
int i;

/* enable pre-emption */
   setPreEmptive(1);
   setIdleHook(idleHook);

/* install the console device */
   if((consoleHandle=installConsole()) < 0)
   {
      printk("Error installing console\r\n");
      return;
   }
   for(i=0; i<25; i++)
   {
      printk("Hello from main\r\n");
      delayTask(1L);
   }
   return;
/* Now create two tasks which attempt to share the console */
   newTask(MINSTACK, appTask, sizeof(int), 0);
   newTask(MINSTACK, appTask, sizeof(int), 1);
}

