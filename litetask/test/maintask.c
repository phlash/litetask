/* MAINTASK.C */

#include "litetask.h"

/* stack size for mainTask() */
int mainStackSize = MINSTACK;

#define N_CLOCKS  5
#define D_TIME    9L

void far clockTimer(long taskID)
{
   resumeTask((taskHandle)taskID, 0);
}

int far clockTask(int clockID)
{
int  x, y, cnt = 0;
timerHandle tim;

/* allocate a timer */
   tim = newTimer();
   if(!tim)
   {
      return 0;
   }

/* run a timer driven clock */
   for(;;)
   {
      for(y=10*clockID+1; y<10*clockID+11; y++)
         for(x=1; x<10; x++) vm18drv.DrawPoint(x, y, (long)cnt, 15L);
      if(++cnt > 15)
         cnt = 0;
      startTimer(tim, (long)clockID+2L, clockTimer, (long)getTaskHandle());
      suspendTask();
   }
   return 0;
}

void far myIdle(void)
{
static int wormX = 0, wormY = 0;
int x;

// Draw a worm running around the screen
   for(x=wormX; x<wormX+20; x++)
      vm18drv.DrawPoint(x, wormY+100, 0L, 15L);
   wormX++;
   if(wormX > 619)
   {
      wormX = 0;
      wormY = (wormY+10) % 380;
   }
   for(x=wormX; x<wormX+20; x++)
      vm18drv.DrawPoint(x, wormY+100, 14L, 15L);
}

void far mainTask(void)
{
taskHandle far *tasks;
int clockID, cnt = 0;

   tasks = (taskHandle far *)calloc( N_CLOCKS, sizeof(taskHandle) );
   if(tasks == NULL)
   {
      return;
   }
   setPreEmptive(1);
   setIdleHook(myIdle);
   vm18drv.InitDriver();
   for(clockID=0; clockID<N_CLOCKS; clockID++)
   {
      tasks[clockID] = newTask(MINSTACK, clockTask, sizeof(int), clockID);
   }
   delayTask(18L * 30L);
   for(clockID=0; clockID<N_CLOCKS; clockID++)
      deleteTask(tasks[clockID]);
   vm18drv.RemoveDriver();
}

