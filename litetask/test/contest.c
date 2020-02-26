#include "litetask.h"
#include "bios.h"
#include "ioctl.h"
#include "select.h"

int mainStackSize = MINSTACK;
semaphore_t sigSem;

void far consoleHi(int handle, int conId)
{
int len, n;
char msg[80]; 

   writeDevice(handle, "\f", 1, 0L);
   len=sprintf(msg, "This is console %i\n", conId);
   writeDevice(handle, msg, len, 0L);
   select(SEL_READ, 1, handle);
   n=readDevice(handle, msg, sizeof(msg), 0L);
   len=sprintf(msg, "Read %i bytes\007\n", n);
   writeDevice(handle, msg, len, 0L);
   putSemaphore(&sigSem);
}

void far idleHook(void)
{
static int cnt = 0;
static char *tw = "-\|/";

   textDrv.OutChXY(79, 0, tw[cnt]);
   cnt = (cnt+1)%4;
}

void far mainTask(char far *cmdLine)
{
int consoles[MAX_CONSOLES],i, cnt = 0;
int doInt15 = 0;
char c;

   setIdleHook(idleHook);
   resetopts();
   while((c = getopts(cmdLine, "iI", NULL)) > 0)
   {
      if(c == 'i' || c == 'I')
         doInt15 = 1;
   }
   if(doInt15);
      installInt15();
   installConsole();
   initSemaphore(&sigSem);
   getSemaphore(&sigSem, NOTIMEOUT);
   for(i=0; i<MAX_CONSOLES; i++)
   {
      consoles[i] = createConsole(i);
      if(consoles[i] >= 0)
      {
         newTask(MINSTACK, consoleHi, 2 * sizeof(int), consoles[i], i);
         cnt++;
      }
   }
   printk("mainTask: waiting for child terminations..\r\n");
   for(i=0; i<cnt; i++)   
      getSemaphore(&sigSem, NOTIMEOUT);
   printk("mainTask: removing devices..\r\n");
   removeConsole();
   if(doInt15)
      removeInt15();
   printk("mainTask: terminating\r\n");
}
