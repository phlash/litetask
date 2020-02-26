/*------------------------------------------------------------------------
   TASK.C - Task Management for LITETASK Kernel

   $Author:   Phlash  $
   $Date:   24 Jun 1995 20:07:50  $
   $Revision:   1.16  $

------------------------------------------------------------------------*/

#include "litetask.h"
#include "kernel.h"
#include "debug.h"

/*
 * bitfield values for task states:
 *      (lower 8 bits are reserved for system libraries)
 */
#define TASK_STATE_BITS 0xFF00

#define RUNNING         0x0000
#define SUSPENDED       0x8000
#define EXITING         0x4000

/* error conditions that can occur in scheduler */

#define NO_TASKS         1
#define INVALID_CONTEXT -1
#define INVALID_STACK   -2
#define MEMORY_FAULT    -3
#define UNKNOWN_ERROR   -127

/* internal types */
typedef struct {
      taskHandle head, tail;
      } taskQ_t;

/* enqueue & dequeue macros */
#define enqueue(queue, task)  {  task->next = NULL; \
                                 task->prev = queue.tail; \
                                 if(queue.tail) \
                                    queue.tail->next = task; \
                                 else \
                                    queue.head = task; \
                                 queue.tail = task; \
                              }

#define dequeue(queue, task)  {  if(task->prev) \
                                    task->prev->next = task->next; \
                                 else \
                                    queue.head = task->next; \
                                 if(task->next) \
                                    task->next->prev = task->prev; \
                                 else \
                                    queue.tail = task->prev; \
                              }

/* internal functions */

void far * far schedule(int irq, void far *oldcontext);
void far exitScheduler(int);
int  far startScheduler(char far *commandTail);
void far beginScheduler(void far *context);
void far taskTimer(long arg);

/* taskInfo_t signature for validation */

static char LTMAGIC[] = "LTsk";

/* internal global data (Kernel ONLY) */

static taskQ_t runQueue = { NULL, NULL };
static taskQ_t delQueue = { NULL, NULL };
static taskHandle currentTask = NULL, idleTask = NULL;
static int nTasks = 0, quitSig = 0;
static short inScheduler = 0, taskLocked = 0;
static int lockOuts = 0, idleSwitches = 0, taskSwitches = 0, scheduleTime = 0;
static timerHandle delayTimer = NULL;
static jmp_buf main_context;

/* ************************************************************************
 * The scheduler itself. 
 * ************************************************************************/
void far * far schedule(int irq, void far *oldcontext)
{
unsigned short flag, schStart, schEnd;

/* check for re-entrance or task lock out */
   flag = lockInts();
   if(inScheduler | taskLocked)
   {
      lockOuts++;
      unlockInts(flag);
      return oldcontext;
   }
   inScheduler = 1;

/* time this re-schedule */
   schStart = getTimer();

/* check if something has bombed the system */
   if (quitSig)
   {
      unlockInts(flag);
      LT_DBG(DBG_KERN_ERROR, "schedule(): quiting on quitSig != 0\r\n");
      exitScheduler(quitSig);
   }

/* check if we have nothing left to run */
   if(nTasks < 2)
   {
      unlockInts(flag);
      LT_DBG(DBG_KERN_EVENT, "schedule(): quitting on nTasks < 2\r\n");
      exitScheduler(NO_TASKS);
   }

/* check validity of task data - bomb on error */
   if( currentTask == NULL ||
      CHKSIG(currentTask->magic, LTMAGIC) )
   {
      unlockInts(flag);
      LT_DBG(DBG_KERN_ERROR, "schedule(): quitting on invalid currentTask\r\n");
      exitScheduler(INVALID_CONTEXT);
   }

/* save old context */
   currentTask->context = oldcontext;

/* put current task back on run queue if necessary */
   if( currentTask != idleTask &&
      (currentTask->taskState & TASK_STATE_BITS) == RUNNING)
      enqueue(runQueue, currentTask);

/* select the next task from the head of the run queue, or idle task */
   if(runQueue.head)
   {
      currentTask = runQueue.head;
      runQueue.head = currentTask->next;
      if(runQueue.head)
         runQueue.head->prev = NULL;
      else
         runQueue.tail = NULL;
      taskSwitches++;
   }
   else
   {
      currentTask = idleTask;
      idleSwitches++;
   }

/* keep running average of schedule time */
   schEnd = getTimer();
   if(schEnd > schStart)
      scheduleTime = scheduleTime/2 + (int)(schEnd - schStart)/2;

/* clear re-entrancy flag on exit */
   inScheduler = 0;
   unlockInts(flag);
   return currentTask->context;
}

/*
 * The scheduler start-up routine
 */
int far startScheduler(char far *commandTail)
{
int i;

/* check for sensible main task stack size and routine address */
   if( mainStackSize < MINSTACK )
      return INVALID_STACK;
   if( mainTask == NULL )
      return INVALID_CONTEXT;

/* allocate and start a task delay timer */
   delayTimer = newTimer();
   if(!delayTimer)
      return MEMORY_FAULT;
   startTimer(delayTimer, 1L, taskTimer, 0L);

/* create context for system termination */
   if(i=setjmp(main_context))
      return i;

/* set up idleTask, stack size MINSTACK */
   idleTask = malloc( sizeof( taskInfo_t ) );
   memcpy(idleTask->magic, LTMAGIC, 4);
   idleTask->stack = malloc( MINSTACK );
   for(i=0; i<MINSTACK; i++)
      ((char far *)(idleTask->stack))[i] = 0x55;
   idleTask->taskState = RUNNING;
   idleTask->extra = 0L;
   idleTask->context = newContext((char far *)idleTask->stack+MINSTACK,
                                 idleTaskFun);
   LT_DBG(DBG_KERN_EVENT, "startScheduler(): created idleTask=");
   LT_DBG(DBG_KERN_EVENT, formatHex(idleTask));

/* set up user task, mainTask(), stack size as defined by user code */
   currentTask = malloc( sizeof( taskInfo_t ) );
   memcpy(currentTask->magic, LTMAGIC, 4);
   currentTask->stack = malloc( mainStackSize );
   for(i=0; i<mainStackSize; i++)
      ((char far *)(currentTask->stack))[i] = 0x55;
   memcpy((char far *)currentTask->stack+mainStackSize-4, &commandTail, 4);
   currentTask->taskState = RUNNING;
   currentTask->extra = 0L;
   currentTask->context = newContext((char far *)currentTask->stack+
                                 mainStackSize-4,
                                 mainTask);
   LT_DBG(DBG_KERN_EVENT, "startScheduler(): created mainTask=");
   LT_DBG(DBG_KERN_EVENT, formatHex(currentTask));

/* tell scheduler we have two tasks */
   nTasks = 2;

/* start things off */
   LT_DBG(DBG_KERN_EVENT, "startScheduler(): invoking mainTask()\r\n");
   beginScheduler(currentTask->context);
/* never gets to here.. */
}

/*
 * The scheduler termination routine
 */
void far exitScheduler(int exitCode)
{
/* say terminating */
   LT_DBG(DBG_KERN_EVENT, "exitScheduler(): returning to main() context\r\n");

/* jump back to main() */
   longjmp(main_context, (exitCode) ? exitCode : UNKNOWN_ERROR);
}

/*
 * quitScheduler() - Used to terminate the system by external code
 */
void far quitScheduler(int exitCode)
{
   quitSig = (exitCode) ? exitCode : UNKNOWN_ERROR;
   yieldTask();
}

/*
 * taskTimer() - times out tasks on the delay queue
 */
void far taskTimer(long arg)
{
taskHandle qEntry, temp;
short flag;

   flag = lockInts();
   for(qEntry = delQueue.head; qEntry; qEntry = temp)
   {
      temp = qEntry->next;
      if(--qEntry->extra <= 0L)
      {
         dequeue(delQueue, qEntry);
         resumeTask(qEntry, (int)qEntry->extra);
      }
   }
   unlockInts(flag);
   startTimer(delayTimer, 1L, taskTimer, 0L);
}

/* ************************************************************************
 * User Interface Functions
 * ************************************************************************/

/*
 * newTask() - create a new task context and place it in the list of
 *             tasks waiting to be run, this creates a new thread
 *             of processing.
 */
taskHandle far newTask(unsigned int stackSize, void far *function,
                     int arg_size, ...)
{
taskHandle anotherTask;
char far *stackEnd;
short flag;
int i;

/* check args */
   if(stackSize < MINSTACK || function == NULL)
      return NULL;

/* allocate a new taskInfo structure */
   anotherTask = malloc( sizeof( taskInfo_t ) );
   if( anotherTask == NULL )
   {
      LT_DBG(DBG_KERN_ERROR, "newTask(): Out of memory\r\n");
      return NULL;
   }
   memcpy(anotherTask->magic, LTMAGIC, 4);
   anotherTask->stack = malloc( stackSize );
   if( anotherTask->stack == NULL )
   {
      free( anotherTask );
      LT_DBG(DBG_KERN_ERROR, "newTask(): Out of memory\r\n");
      return NULL;
   }
   for(i=0; i<stackSize; i++)
      ((char far *)(anotherTask->stack))[i] = 0x55;

/* transfer function arguments to new stack */
   stackEnd = (char far *)(anotherTask->stack) + stackSize;
   stackEnd -= arg_size;
   for(i=0; i<arg_size; i++)
      stackEnd[i] = *((char *)&arg_size + sizeof(int) + i);

/* create dummy context for task startup */
   anotherTask->context = newContext(stackEnd, function);

/* append to run queue & initialise task state */
   LT_DBG(DBG_KERN_EVENT, "newTask(): created task=");
   LT_DBG(DBG_KERN_EVENT, formatHex(anotherTask));
   flag = lockInts();
   anotherTask->taskState = RUNNING;
   anotherTask->extra = 0L;
   enqueue(runQueue, anotherTask);
   nTasks++;
   unlockInts(flag);

/* return the new task handle */
   return anotherTask;
}

/*
 * taskExit() - automatically called on task 'return', or called directly
 *              from the terminating task
 */
void far taskExit(int exitStatus)
{
short flag;

/* set task state to EXITING and enter scheduler */
   LT_DBG(DBG_KERN_EVENT, "taskExit(): exiting task=");
   LT_DBG(DBG_KERN_EVENT, formatHex(currentTask));
   flag = lockInts();
   currentTask->extra = (long)exitStatus;
   currentTask->taskState = EXITING;
   nTasks--;
   unlockInts(flag);
   yieldTask();
}

/*
 * deleteTask() - Deletes a task immediately. NOTE: You cannot delete the
 *                current task (yourself).
 */
int far deleteTask(taskHandle taskID)
{
short flag;

/* check taskID */
   if( taskID == currentTask )
      return ETASKID;

/* check magic */
   if(CHKSIG(taskID->magic, LTMAGIC))
      return ETASKID;

/* remove task from run queue if necessary */
   LT_DBG(DBG_KERN_EVENT, "deleteTask(): deleting task=");
   LT_DBG(DBG_KERN_EVENT, formatHex(taskID));
   flag = lockInts();
   if((taskID->taskState & TASK_STATE_BITS) == RUNNING)
      dequeue(runQueue, taskID);

   if(!(taskID->taskState & EXITING))
      nTasks--;
   unlockInts(flag);

/* invalidate magic field - anti-hacking device ! */
   taskID->magic[0] = 0;

/* remove memory structures */
   free( taskID->stack );
   free( taskID );
   return 0;
}

/*
 * waitTask() - Waits for the specified task to exit, timing out
 *              after the specified number of ticks. If the
 *              number of ticks is -ve, wait forever.
 *              NOTE: you CANNOT wait on your own termination!
 */
int far waitTask(taskHandle taskID, long waitTicks)
{
long startTick;
int exitStatus;

/* check task ID */
   if( taskID == currentTask )
      return ETASKID;

/* check magic */
   if(CHKSIG(taskID->magic, LTMAGIC))
      return ETASKID;

/* wait for specified number of ticks, checking task state */
   startTick = getTicks();   
   while( !(taskID->taskState & EXITING) )
   {
      if( waitTicks >= 0 )
      {
         if( getTicks() >= (startTick + waitTicks) )
         {
            LT_DBG(DBG_KERN_EVENT, "waitTask(): timed out on task=");
            LT_DBG(DBG_KERN_EVENT, formatHex(taskID));
            return ETIMEOUT;
         }
      }
      yieldTask();
   }

/* grab exit status */
   exitStatus = (int)taskID->extra;

/* delete the task control data */
   deleteTask(taskID);

/* return exit status */
   LT_DBG(DBG_KERN_EVENT, "waitTask(): picked up task=");
   LT_DBG(DBG_KERN_EVENT, formatHex(taskID));
   return exitStatus;
}

/*
 * getTaskHandle() - returns current task pointer 
 */
taskHandle far getTaskHandle(void)
{
   return currentTask;
}

/*
 * lockTask() - forces scheduler to run this task ONLY
 */
short far lockTask(void)
{
short rv, flag; 

   flag = lockInts();
   rv = taskLocked++;
   unlockInts(flag);
   return rv;
}

/*
 * unlockTask() - reverses lockTask()
 */
void far unlockTask(short flag)
{
   taskLocked = flag;
}

/*
 * yieldTask() - forces scheduler to switch to next task in run list,
 * however, this could be the current task (if there is only one!)
 */
void far yieldTask(void)
{
/* Clear any interrupt and task locks (or this call is pointless!) */
   enableInts();
   taskLocked = 0;

/* Call the scheduler */
   scheduleEntry();
}

/*
 * suspendTask() - suspends the current task forever, until another
 * task issues the resumeTask() call, and supplies a return value.
 */
int far suspendTask(void)
{
short flag; 

/* set the current task to a suspended state and yield */
   flag = lockInts();
   currentTask->taskState |= SUSPENDED;
   unlockInts(flag);
   yieldTask();

/* return reason for waking up */
   return (int)currentTask->extra;
}

/*
 * resumeTask() - restore the specified task to run status
 */
int far resumeTask( taskHandle taskID, int resumeStatus )
{
short flag;

/* check arguments */
   if(CHKSIG(taskID->magic, LTMAGIC))
      return ETASKID;

   flag = lockInts();
   if(taskID->taskState & SUSPENDED)
   {
      taskID->taskState &= ~SUSPENDED;
      taskID->extra = (long)resumeStatus;
      enqueue(runQueue, taskID);
      unlockInts(flag);
      return 0;
   }
   unlockInts(flag);
   return ETASKSTATE;
}

/*
 * delayTask() - delays the current task for the specified number of
 * clock ticks
 */
int far delayTask(long delayTicks)
{
short flag;

/* append this task to the delay queue, then suspend it */
   flag = lockInts();
   currentTask->extra = delayTicks;
   enqueue(delQueue, currentTask);
   unlockInts(flag);
   return suspendTask();
}

/*
 * getTaskCount() - returns current number of active tasks
 */
int far getTaskCount(void)
{
   return nTasks;
}

/*
 * getLockOuts() - returns current number of scheduler lock-outs
 */
int far getLockOuts(void)
{
   return lockOuts;
}

/*
 * getTaskSwitches() - returns current number of task switchs
 */
int far getTaskSwitches(void)
{
   return taskSwitches;
}

/*
 * getIdleSwitches() - returns current number of idle switchs
 */
int far getIdleSwitches(void)
{
   return idleSwitches;
}

/*
 * getScheduleTime() - Read duration of last schedule operation
 */
int far getScheduleTime(void)
{
   return scheduleTime;
}

/*
 * traceTaskStack() - see how much stack is unused for a task
 */
int far traceTaskStack( taskHandle taskID )
{
int i;

/* check arguments */
   if(CHKSIG(taskID->magic, LTMAGIC))
      return ETASKID;

/* see how much of the stack is at the initial value of 0x5555 */
   for(i=0; i<MAXSTACK/2; i++)
      if( ((int far *)(taskID->stack))[i] != 0x5555 )
         break;
   return 2*i;
}

/* End */
