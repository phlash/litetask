#include "litetask.h"
#include "debug.h"

int mainStackSize = MINSTACK;
semaphore_t myLock;

void far subTask(int id)
{
taskHandle me;
long to = 18L;
int rv, attempts = 0, gots = 0;
char buf[22];

   me = getTaskHandle();
   sprintf(buf, "%x:%x", FP_SEG(me), FP_OFF(me));
   textDrv.OutStrXY(0, 6+id, buf);
   textDrv.OutStrXY(40, 6+id, "Sub: Get semaphore");
   for(;;)
   {
      switch(rv=getSemaphore(&myLock, to))
      {
      case 0:
         textDrv.OutStrXY(40, 6+id, "Sub: Got semaphore  *");
         gots++;
         delayTask(2L);
         if(putSemaphore(&myLock))
         {
            textDrv.OutStrXY(40, 6+id, "Sub: Cannot put semaphore");
            return;
         }
         textDrv.OutStrXY(40, 6+id, "Sub: Put semaphore   ");
         break;
      case EBADSEM:
         textDrv.OutStrXY(40, 6+id, "Sub: Bad semaphore");
         return;
      case ESEMINUSE:
         textDrv.OutStrXY(40, 6+id, "Sub: Lck semaphore");
         break;
      case ESEMTIMEOUT:
         textDrv.OutStrXY(40, 6+id, "Sub: Tim semaphore");
         sprintf(buf, "To: %i", ++to);
         textDrv.OutStrXY(30, 6+id, buf);
         break;
      default:
         sprintf(buf, "Sub: Oops: %i  ", rv);
         textDrv.OutStrXY(40, 6+id, buf);
         return;
      }
      attempts++;
      sprintf(buf, "Atts: %i Gots: %i", attempts, gots);
      textDrv.OutStrXY(10, 6+id, buf);
      sprintf(buf, "Stack: %i", traceTaskStack(me));
      textDrv.OutStrXY(65, 6+id, buf);
   }
}

void far myIdle(void)
{
static taskHandle oldOwner = (taskHandle)-1;
char buf[40];

   if(myLock.owner != oldOwner)
   {
      oldOwner = myLock.owner;
      sprintf(buf, "myLock.owner: %x:%x  ", FP_SEG(oldOwner), FP_OFF(oldOwner));
      textDrv.OutStrXY(0, 4, buf);
   }
}

void far breakTrap(void)
{
   quitScheduler(1);
}

void far mainTask(char far *commandLine)
{
int i;
char buf[80];

   LT_DBG(DBG_USR0_EVENT, "mainTask(): started!\r\n");
   textDrv.Init();
   setPrintk(&textDrv, NULL, 0);
   setPreEmptive(10);
   installTimer(11930);
   setIdleHook(myIdle);
   setBreakKeyTrap(breakTrap);
   initSemaphore(&myLock);
   printk("mainTask: delaying for 10 seconds\r\n");
   delayTask(1000L);
   for(i=0; i<15; i++)
      newTask(MINSTACK, subTask, sizeof(int), i);
   for(;;)
   {
      sprintf(buf, "Main: Idles %x Tasks %x     ",
         getIdleSwitches(), getTaskSwitches());
      textDrv.OutStrXY(0, 5, buf);
      delayTask(100L);
   }
}
