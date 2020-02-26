/*------------------------------------------------------------------------
   TIMER.C - LiteTask Kernel timer management functions

   $Author:   Phlash  $
   $Date:   24 Jun 1995 20:11:08  $
   $Revision:   1.7  $

------------------------------------------------------------------------*/

#include "litetask.h"
#include "kernel.h"
#include "8259.h"
#include "8253.h"
#include "debug.h"

/* local functions */
int far clockTick(int irq, debugRegs_t far *intContext);

/* Installed flag */
static int installed=0;

/* Unique ID code for testing pointers */
static char TIMER_MAGIC[] = "LT-T";

/* global link list of timer structures */
static timerHandle timerList = NULL;

/* global system tick counter */
static volatile long ticks = 0L;

/* global timer re-load value */
static unsigned short timerMax=65535;

/* global timer counter */
static unsigned short timerCount=0;

/* pre-emption tick count (default is non-preemptive) */
static int preEmptive = 0;

/* profiling address, buffer & length */
static DWORD pAddr = 0L;
static WORD far *pBuf = NULL;
static WORD pBufLen = 0;

/* stack for timer interrupts */
static char clockStack[MINSTACK];

/* ****** Functions ****** */

/*
 * installTimer() - install timer with specified period
 */
int far installTimer(unsigned short timerCnt)
{
short flag;

   flag = lockInts();
   if(!installed)
   {
      installed = 1;
      timerCount = 0;
      ticks = 0L;
      memset(clockStack, 0x55, sizeof(clockStack));
      setIRQTrap(0, clockTick, clockStack+sizeof(clockStack));
   }
   timerMax=timerCnt;
   outp(TMR_CNTL_PORT, TMR0_CNTL | TMR_MODE2);
   outp(TMR0_CNTR_PORT, (unsigned char)timerMax);
   outp(TMR0_CNTR_PORT, (unsigned char)(timerMax >> 8));
   unlockInts(flag);
   return 0;
}

/*
 * removeTimer() - unhook the timer IRQ
 */
void far removeTimer(void)
{
short flag;

   flag = lockInts();
   if(installed)
   {
      installed = 0;
      outp(TMR_CNTL_PORT, TMR0_CNTL | TMR_MODE2);
      outp(TMR0_CNTR_PORT, 0xFF);
      outp(TMR0_CNTR_PORT, 0xFF);
      clearIRQTrap(0);
   }
   unlockInts(flag);
}

/*
 * getTicks() - read tick counter
 */
long far getTicks(void)
{
short flag;
long rval;

   flag = lockInts();
   rval = ticks;
   unlockInts(flag);
   return rval;
}

/*
 * getTimer() - read 8253 timer value
 */
unsigned short far getTimer(void)
{
unsigned short timer;
short flag;

   flag = lockInts();
   outp(TMR_CNTL_PORT, TMR0_CNTL | TMR_LATCH);
   timer = inp(TMR0_CNTR_PORT);
   timer |= inp(TMR0_CNTR_PORT) << 8;
   unlockInts(flag);
   return (timerMax-timer);
}

/*
 * clockTick() - called on hardware clock interrupts, to provide tasks with
 * accurate hardware tick timers.
 */
int far clockTick(int irq, debugRegs_t far *intContext)
{
timerHandle timer;
DWORD intAddr;

/* disable interrupts */
   disableInts();

/* increment tick counter */
   ticks++;

/* store profiling data */
   if(pBuf)
   {
      intAddr = ((DWORD)(intContext->CS) << 4) + (DWORD)intContext->IP;
      if(intAddr > pAddr && (WORD)(intAddr - pAddr) < pBufLen)
         pBuf[(WORD)(intAddr - pAddr)]++;
   }

/* call BIOS if timerCount will wrap around 65536 */
   if(timerCount+timerMax+1 <= timerCount)
      chainIRQ(irq);
   else
      outp(INT_CNTL_PORT, EOI);
   timerCount=timerCount+timerMax+1;

/* enable interrupts again */
   enableInts();

/* decrement all ACTIVE watchdog counters */
   for(timer=timerList; timer!=NULL; timer=timer->next)
   {
   /* if timed out, then clear status and call function, 
    * NB: It is important to clear status BEFORE calling function,
    * since the function may wish to re-install itself..
    */
      if(timer->status == WDT_ACTIVE)
      {
         if(--(timer->count) <= 0L)
         {
            timer->status = WDT_DONE;
            (*(timer->function))(timer->arg);
         }
      }
   }

/* if pre-emption is required, return non-zero */
   if(!preEmptive)
      return 0;
   return !(ticks%preEmptive);
}

/* newTimer() - creates a new watchdog timer in the linked list */
timerHandle far newTimer(void)
{
timerHandle timer;
short flag;

/* allocate new structure and fill it in */
   timer = malloc( sizeof(timerInfo_t) );
   if(timer == NULL)
   {
      LT_DBG(DBG_KERN_ERROR, "newTimer(): Out of memory\r\n");
      return NULL;
   }

   memcpy(timer->magic, TIMER_MAGIC, 4);
   timer->status   = WDT_DONE;
   timer->count    = 0L;
   timer->function = NULL;
   timer->arg      = 0L;

/* now lock out interrupts and add timer to head of list */
   flag = lockInts();
   timer->next = timerList;
   timerList = timer;
   unlockInts(flag);

/* return pointer to allocated timer structure */
   LT_DBG(DBG_KERN_EVENT, "newTimer(): timer=");
   LT_DBG(DBG_KERN_EVENT, formatHex(timer));
   return timer;
}

/*
 * startTimer() - starts a timer off
 */
int far startTimer(timerHandle timer, long delay,
               void (far *function)(long), long argument)
{
short flag;

/* check timer ID is valid */
   if(timer == NULL)
      return ETIMERID;
   if(CHKSIG(timer->magic, TIMER_MAGIC))
      return ETIMERID;

/* fill out info and start timer */
   flag = lockInts();
   timer->count    = delay;
   timer->function = function;
   timer->arg      = argument;
   timer->status   = WDT_ACTIVE;
   unlockInts(flag);
   return 0;
}

/*
 * stopTimer() - stops a timer (running or not)
 */
int far stopTimer(timerHandle timer)
{
short flag;

/* check timer ID is valid */
   if(timer == NULL)
      return ETIMERID;
   if(CHKSIG(timer->magic, TIMER_MAGIC))
      return ETIMERID;

/* now stop it */
   flag = lockInts();
   timer->status = WDT_DONE;
   unlockInts(flag);
   return 0;
}

/*
 * deleteTimer() - stops and removes a timer
 */
int far deleteTimer(timerHandle timer)
{
timerHandle lastp, prevp;
short flag, done = 0;

/* attempt to stop timer first (also checks ID) */
   if(stopTimer(timer))
      return ETIMERID;

/* lock interrupts and search list for timer */
   flag = lockInts();
   prevp = NULL;
   for(lastp=timerList; lastp!=NULL; lastp=lastp->next)
   {
      if(lastp == timer)
      {
      /* unlink it from the list */
         if(prevp)
            prevp->next = timer->next;
         else
            timerList = timer->next;
         done = 1;
         break;
      }
      prevp = lastp;
   }
   unlockInts(flag);

/* search complete, check we found timer OK */
   if(!done)
      return ETIMERID;
   LT_DBG(DBG_KERN_EVENT, "deleteTimer(): timer=");
   LT_DBG(DBG_KERN_EVENT, formatHex(timer));
   free(timer);
   return 0;
}

/*
 * setPreEmptive() - Enables / disables task pre-emption by hardware clock
 */
void far setPreEmptive(int ticks)
{
   LT_DBG(DBG_KERN_EVENT, "setPreEmptive(): ");
   preEmptive = ticks;
   LT_DBG(DBG_KERN_EVENT, (preEmptive) ? "enabled\r\n" : "disabled\r\n");
}

/*
 * enableProf() - Enables profiling at specified address in specified buffer
 */
int far enableProf(void far *addr, WORD far *buf, WORD len)
{
short flag;

/* Sanity check */
   if(!addr || !buf || !len)
      return -1;

/* store profiling info */
   flag = lockInts();
   pAddr = ((DWORD)(FP_SEG(addr)) << 4) + (DWORD)(FP_OFF(addr));
   pBuf = buf;
   pBufLen = len;
   unlockInts(flag);
   return 0;
}

/*
 * disableProf() - Stops profiling
 */
void far disableProf(void)
{
short flag;

   flag = lockInts();
   pAddr = 0L;
   pBuf = NULL;
   pBufLen = 0;
   unlockInts(flag);
}

/* End */

