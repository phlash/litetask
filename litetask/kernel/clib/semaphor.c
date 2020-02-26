/*------------------------------------------------------------------------
   SEMAPHOR.C - Semaphore operations for LiteTask Kernel

   $Author:   Phlash  $
   $Date:   26 Mar 1995 14:30:44  $
   $Revision:   1.6  $

------------------------------------------------------------------------*/

#include "litetask.h"
#include "debug.h"

#define SEM_AQUIRED  0
#define SEM_TIMEOUT  1

/* internal prototypes */
static void far _semaphoreTimer(long ptr);
static void far _runTimer(void);

/* magic number to check pointers */
static char SEM_MAGIC[] = "LT-S";

/* timer initialisation flag */
static int timerRunning = 0;

/* list of pending timeouts */
static semQueue_t far *timeoutList = NULL;

/*
 * initSemaphore() - Initialises a semaphore structure
 */
void far initSemaphore(semaphore_t far *newSem)
{
short flag;

/* check for timer initialised */
   if(!timerRunning)
      _runTimer();

/* lock interrupts */
   flag = lockInts();

/* fill out semaphore */
   memcpy(newSem->magic, SEM_MAGIC, 4);
   newSem->owner = NULL;
   newSem->qhead = NULL;
   newSem->qtail = NULL;

/* unlock interrupts */
   unlockInts(flag);
}

/*
 * getSemaphore() - Aquires ownership of a semaphore
 */
int far getSemaphore(semaphore_t far *sem, long timeout)
{
int rv;
short flag;
semQueue_t queueEntry;

/* check timer is running */
   if(!timerRunning)
      _runTimer();
   if(!timerRunning)
      return EINTERNAL;

/* check semaphore is valid */
   if(memcmp(sem->magic, SEM_MAGIC, 4))
      return EBADSEM;

/* anybody got this one? */
   flag = lockInts();
   if(sem->owner == NULL)
   {
      sem->owner = getTaskHandle();
      unlockInts(flag);
      return 0;
   }

/* can't wait? Oh well then.. */
   if(!timeout)
   {
      unlockInts(flag);
      return ESEMINUSE;
   }

/* place on end of waiting queue */
   queueEntry.task = getTaskHandle();
   queueEntry.psem = sem;
   queueEntry.qnext = NULL;
   if(sem->qtail)
   {
      sem->qtail->qnext = &queueEntry;
      queueEntry.qprev = sem->qtail;
   }
   else
   {
      sem->qhead = &queueEntry;
      queueEntry.qprev = NULL;
   }
   sem->qtail = &queueEntry;

/* shall we start a timer? */
   if(timeout > 0L)
   {
      queueEntry.remaining = timeout;
      queueEntry.tnext = timeoutList;
      queueEntry.tprev = NULL;
      if(timeoutList)
         timeoutList->tprev = &queueEntry;
      timeoutList = &queueEntry;
   }

/* now suspend this task until we timeout or aquire the semaphore.
   NB: suspendTask() will release the interrupt lock atomically. */
   switch(rv=suspendTask())
   {
   case SEM_AQUIRED:
      rv = 0;
      break;
   case SEM_TIMEOUT:
      return ESEMTIMEOUT;
   default:
      LT_DBG(DBG_KERN_ERROR, "getSemaphore: odd return value from suspendTask()!\r\n");
      break;
   }

/* remove entry from timeout pending list */
   if(timeout > 0L)
   {
      flag = lockInts();
      if(queueEntry.tprev)
         queueEntry.tprev->tnext = queueEntry.tnext;
      else
         timeoutList = queueEntry.tnext;
      if(queueEntry.tnext)
         queueEntry.tnext->tprev = queueEntry.tprev;
      unlockInts(flag);
   }
   return rv;
}

/*
 * putSemaphore() - Releases a semaphore
 */
int far putSemaphore(semaphore_t far *sem)
{
semQueue_t far *next;
short flag;

/* check timer is running */
   if(!timerRunning)
      _runTimer();
   if(!timerRunning)
      return EINTERNAL;

/* check semaphore is valid */
   if(memcmp(sem->magic, SEM_MAGIC, 4))
      return EBADSEM;

/* lock interrupts */
   flag = lockInts();

/* now see if we need to pass on the semaphore */
   if(sem->qhead)
   {
      next = sem->qhead;
      sem->owner = next->task;
      resumeTask(next->task, SEM_AQUIRED);
      if(next->qnext)
         next->qnext->qprev = NULL;
      else
         sem->qtail = NULL;
      sem->qhead = next->qnext;
   }
   else
      sem->owner = NULL;

/* unlock interrupts */
   unlockInts(flag);
   return 0;
}

void far _semaphoreTimer(long ptr)
{
semQueue_t far *next;
short flag;

/*
   Decrement and process timeouts on zero
*/
   flag = lockInts();
   for(next = timeoutList; next != NULL; next = next->tnext)
   {
      if(--(next->remaining) <= 0L)
      {
         if(resumeTask(next->task, SEM_TIMEOUT))
            continue;
         if(next->tprev)
            next->tprev->tnext = next->tnext;
         else
            timeoutList = next->tnext;
         if(next->tnext)
            next->tnext->tprev = next->tprev;
         if(next->qprev)
            next->qprev->qnext = next->qnext;
         else
            next->psem->qhead = next->qnext;
         if(next->qnext)
            next->qnext->qprev = next->qprev;
         else
            next->psem->qtail = next->qprev;
      }
   }
   unlockInts(flag);
/*
   Re-load timer
*/
   startTimer((timerHandle)ptr, 1L, _semaphoreTimer, ptr);
}

void far _runTimer(void)
{
timerHandle semTimer;
short flag;

/* lock this task */
   flag = lockTask();

/* allocate a timer */
   semTimer = newTimer();
   if(!semTimer)
   {
      unlockTask(flag);
      LT_DBG(DBG_CLIB_ERROR, "_runTimer: Cannot allocate semaphore timer\r\n");
      return;
   }

/* set timer to run on next clock tick */
   startTimer(semTimer, 1L, _semaphoreTimer, (long)semTimer);
   timerRunning = 1;
   unlockTask(flag);
}

/* End. */
