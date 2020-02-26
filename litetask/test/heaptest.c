/* HEAPTEST.C */

#include "litetask.h"

extern void srand(unsigned);
extern int rand(void);

/* stack size for mainTask() */
int mainStackSize = MINSTACK;

void far idleFun(void)
{
heapInfo_t heap;
unsigned short flag, count;
unsigned long total;
char buf[81];
static int cnt = 0;

/* return on 999 out of 1000 calls */
   cnt = (cnt+1) % 1000;
   if(cnt)
      return;

/* clear the display area used */
   for(count=0; count<80; count++)
      buf[count] = ' ';
   buf[count] = 0;
   for(count=4; count<24; count++)
      textDrv.OutStrXY(0, count, buf);

/* walk the heap and display info */
   flag = lockTask();
   heap.mcb = NULL;
   count = 0;
   total = 0L;
   do
   {
      if(walkHeap(&heap))
      {
         textDrv.OutStrXY(0,3,"Heap trashed!");
         unlockTask(flag);
         return;
      }
      sprintf(buf, "B:%i S:%l U/L:%i/%i",
         count, heap.size, heap.used, heap.last);
      textDrv.OutStrXY((count/20)*20,4+(count%20),buf);
      total += heap.size;
      count++;
   } while(heap.mcb);
   sprintf(buf, "Total=%l", total);
   textDrv.OutStrXY(0,3,buf);

/* walk the free list and display info */
   heap.mcb = NULL;
   count = 0;
   do
   {
      if(walkFree(&heap))
      {
         textDrv.OutStrXY(40,3,"Free trashed!");
         unlockTask(flag);
         return;
      }
      sprintf(buf, "B:%i S:%l U/L:%i/%i",
         count, heap.size, heap.used, heap.last);
      textDrv.OutStrXY(40+(count/20)*20,4+(count%20),buf);
      count++;
   } while(heap.mcb);
   unlockTask(flag);
}

#ifdef RANDOM_TEST
void far mainTask(void)
{
int i;
char far *bufs[2];

/* allocate 2 buffers */
   setPreEmptive(1);
   setIdleHook(idleFun);
   srand(getTicks());
   textDrv.Init();
   bufs[0] = (char far *)malloc(rand());
   bufs[1] = (char far *)malloc(rand());
   textDrv.OutStrXY(0,24,"Allocated");
   delayTask(36L);

/* now allocate & free at random! */
   for(;;)
   {
      i = rand()/16384;
      if(bufs[i])
      {
         free(bufs[i]);
         bufs[i] = NULL;
      }
      else
         bufs[i] = (char far *)malloc(rand());
      delayTask(2L);
   }
}
#else
void far mainTask(void)
{
int i;
char far *bufs[50];

/* allocate until exhausted, then free again */
   setPreEmptive(1);
   setIdleHook(idleFun);
   srand(getTicks());
   textDrv.Init();
   for(;;)
   {
      if(tidyHeap())
         return;
      delayTask(36L);
      i=0;
      while(bufs[i] = (char far *)malloc(24576))
      {
         i++;
         delayTask(2L);
      }
      while(--i >= 0)
      {
         free(bufs[i]);
         delayTask(2L);
      }
   }
}
#endif
