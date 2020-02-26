/*------------------------------------------------------------------------
   IDLE.C - LiteTask Idle Function

   $Author:   Phlash  $
   $Date:   24 Jun 1995 20:22:54  $
   $Revision:   1.0  $

------------------------------------------------------------------------*/
#include "litetask.h"
#include "debug.h"
#include "kernel.h"

/*
 * The list of idle functions to call out
 */
static void (far *idleHooks[MAX_IDLEHOOKS])(void) = { NULL };
static semaphore_t idleSem;
static int init=0;

/*
 * Idle function, runs when no other tasks are runnable.
 * NB: This task must never suspend itself!
 */
void far idleTaskFun(void)
{
int i;
short flag;

   LT_DBG(DBG_KERN_EVENT, "idleTaskFun(): started\r\n");
   flag = lockTask();
   if(!init)
      initSemaphore(&idleSem);
   init=1;
   unlockTask(flag);
   for(;;)
   {
   /* aquire the semaphore (busy-wait) */
      while(getSemaphore(&idleSem, 0L))
      {
         LT_DBG(DBG_KERN_EVENT, "idleTaskFun(): failed to aquire semaphore\r\n");
         yieldTask();
      }

   /* run idle hooks */
      for(i=0; i<MAX_IDLEHOOKS; i++)
      {
         if(idleHooks[i])
            (*idleHooks[i])();
      }

   /* release semaphore */
      putSemaphore(&idleSem);

   /* yield at least once on each loop round */
      yieldTask();
   }
}

/*
 * setIdleHook() - adds function to idle task loop
 */
int far setIdleHook(void (far *hookFunction)(void))
{
short flag;
int i;

/* check for valid pointer? */
   if(hookFunction == NULL)
      return EARGS;

/* add to list of functions */
   flag = lockTask();
   if(!init)
      initSemaphore(&idleSem);
   init=1;
   unlockTask(flag);
   if(getSemaphore(&idleSem, NOTIMEOUT))
      return EINTERNAL;

   for(i=0; i<MAX_IDLEHOOKS; i++)
      if(idleHooks[i] == NULL)
         break;
   if(i >= MAX_IDLEHOOKS)
   {
      putSemaphore(&idleSem);
      LT_DBG(DBG_KERN_ERROR, "setIdleHook(): No hook space left\r\n");
      return ENOIDLESPACE;
   }
   idleHooks[i] = hookFunction;
   putSemaphore(&idleSem);
   LT_DBG(DBG_KERN_EVENT, "setIdleHook(): Added hook=");
   LT_DBG(DBG_KERN_EVENT, formatHex(hookFunction));
   return 0;
}

int far clearIdleHook(void (far *hookFunction)(void))
{
int i;
short flag;

/* delete from list of functions */
   flag = lockTask();
   if(!init)
      initSemaphore(&idleSem);
   init=1;
   unlockTask(flag);
   if(getSemaphore(&idleSem, NOTIMEOUT))
      return EINTERNAL;

   for(i=0; i<MAX_IDLEHOOKS; i++)
      if(idleHooks[i] == hookFunction)
         break;
   if(i >= MAX_IDLEHOOKS)
   {
      putSemaphore(&idleSem);
      LT_DBG(DBG_KERN_ERROR, "clearIdleHook(): Cannot find function\r\n");
      return EARGS;
   }
   idleHooks[i] = NULL;
   putSemaphore(&idleSem);
   LT_DBG(DBG_KERN_EVENT, "clearIdleHook(): Removed hook=");
   LT_DBG(DBG_KERN_EVENT, formatHex(hookFunction));
   return 0;
}

