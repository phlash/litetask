/*------------------------------------------------------------------------
   KERNTEST.C - LiteTask Kernel Tests

   $Author:$
   $Date:$
   $Revision:$

   Notes:
   1) This code assumes that the LiteTask C-Library routines work OK,
      particularly printk() / sprintf().
------------------------------------------------------------------------*/
#include <litetask.h>

/* Memory address to store printk output for debug :-) */
#define MSG_ADDR     0x90000000
#define MSG_SIZE     0x7FFF

/* 80x86 opCode byte for INT 3h (breakpoint) */
#define BREAKPOINT   0xCC;

/* Kernel panic() function - displays a message then reboots the machine
   XXXX - Maybe this should be in litetask.h? */
extern void far panic(char far *);

/* Gash screen output function for X/Y postitioned text, we can't use the
   built-in text driver yet... */

#define MDA_ADDR  0xB0000000
#define CGA_ADDR  0xB8000000
static int far *screen = (int far *)CGA_ADDR;

static void outStrXY(int X, int Y, char far *str)
{
int i;

   for(i=0; str[i] != '\0'; i++)
      screen[(Y*80)+X+i] = ((int)str[i] & 0xFF) | 0x0700;
}


int far subTaskFunction(int arg)
{
   printk("subTaskFunction: arg=%i tasks=%i\r\n", arg, getTaskCount());
   return arg;
}

void far myDelay(int cnt)
{
   printk("myDelay%i:in ", cnt);
   delayTask((long)cnt);
   printk("myDelay%i:out ", cnt);
}

void far myIdle(void)
{
char buf[20];
static int idles=0;

   sprintf(buf, "Idle: %i  ", idles);
   idles = (idles+1)%1000;
   outStrXY(68, 0, buf);
}

int taskTests(void)
{
taskHandle subTask;
int i;
long l;

/* Test 1: task creation / round-robin scheduling / deletion */

   printk("taskTests: creating new thread\r\n");
   subTask = newTask(MINSTACK, subTaskFunction, sizeof(int), 1234);
   if(!subTask)
      return 1;
   printk("taskTests: switching to new task..");
   yieldTask();
   printk("taskTests: recovering task exit code..");
   printk("%i\r\n", i=waitTask(subTask, NOTIMEOUT));
   if(i != 1234)
      return 1;
   printk("taskTests: schedule time was %i usecs\r\n", getScheduleTime());
   printk("taskTests: current switching stats: ");
   printk("L:%i T:%i I:%i\r\n", getLockOuts(), getTaskSwitches(), getIdleSwitches());

/* Test 2: delay-queuing / run-queuing logic */
   printk("taskTests: delaying for 18 ticks..");
   delayTask(18L);
   printk("done\r\n");
   printk("taskTests: creating 5 delay threads\r\n");
   for(i=0; i<5; i++)
      newTask(MINSTACK, myDelay, sizeof(int), i+5);
   printk("taskTests: delaying for 18 ticks..");
   delayTask(18L);
   printk("taskTests: av. schedule time was %i usecs\r\n", getScheduleTime());
   printk("taskTests: current switching stats: ");
   printk("L:%i T:%i I:%i\r\n", getLockOuts(), getTaskSwitches(), getIdleSwitches());

/* Test 3: Idle hook operation */
   printk("taskTests: installing idle hook\r\n");
   if(setIdleHook(myIdle))
      return 1;
   printk("taskTests: delaying for 18 ticks..");
   delayTask(18L);
   printk("\r\ntaskTests: clearing idle hook\r\n");
   if(clearIdleHook(myIdle))
      return 1;

/* Test 4: Pre-emption mechanism */
   printk("taskTests: creating new thread\r\n");
   subTask = newTask(MINSTACK, subTaskFunction, sizeof(int), 4321);
   if(!subTask)
      return 1;
   printk("taskTests: enabling pre-emption\r\n");
   setPreEmptive(1);
   printk("taskTests: busy-waiting for 18 ticks..");
   l = getTicks();
   while(getTicks() < l+18L);
   printk("taskTests: disabling pre-emption\r\n");
   setPreEmptive(0);
   printk("taskTests: recovering task exit code..");
   printk("%i\r\n", i=waitTask(subTask, NOTIMEOUT));
   if(i != 4321)
      return 1;
   printk("taskTests: av. schedule time was %i usecs\r\n", getScheduleTime());
   printk("taskTests: current switching stats: ");
   printk("L:%i T:%i I:%i\r\n", getLockOuts(), getTaskSwitches(), getIdleSwitches());

/* Test 5: Check my stack depth */
   printk("taskTests: unused stack depth=%i\r\n", traceTaskStack(getTaskHandle()));

/* All done */
   printk("taskTests: Completed OK\r\n");
   return 0;
}

void interrupt far BIOSClock(void)
{
char buf[20];
static int bticks=0;

   sprintf(buf, "BIOS: %i  ", bticks);
   bticks = (bticks+1)%1000;
   outStrXY(68, 2, buf);
}

void far myTimer(long arg)
{
char buf[20];
static int tticks=0;

   startTimer((timerHandle)arg, 1L, myTimer, arg);
   sprintf(buf, "Tick: %i  ", tticks);
   tticks = (tticks+1)%1000;
   outStrXY(68, 1, buf);
}

WORD profBuf[20000];

int timerTests(int speed)
{
void far *oldBClk;
timerHandle myTim;
int i;
WORD profIdx, profMax, profMaxIdx;

/* Test 1: timer creation / startup */
   printk("timerTests: creating local timer\r\n");
   myTim = newTimer();
   if(!myTim)
      return 1;
   startTimer(myTim, 1L, myTimer, (long)myTim);

/* Test 2: BIOS clock check */
   printk("timerTests: trapping BIOS clock\r\n");
   oldBClk = setVector(0x1C, BIOSClock);
   printk("timerTests: delaying for 36 ticks..");
   delayTask(36L);
   printk("done\r\n");

/* Test 3: Timer re-programming */
   printk("timerTests: setting timer to %iHz\r\n", speed);
   installTimer((int)(1193000L/(long)speed));
   printk("timerTests: delaying for %i ticks..", speed*2);
   delayTask((long)speed*2L);
   printk("done\r\n");
   printk("timerTests: re-setting timer to default rate\r\n");
   installTimer(65535);

/* Test 4: Execution Profilier */
   printk("timerTests: enabling profiler\r\n");
   memset(profBuf, 0, sizeof(profBuf));
   if(enableProf(outStrXY, profBuf, sizeof(profBuf)/sizeof(WORD)))
      return 1;
   printk("timerTests: poll waiting for 180 ticks..");
   for(i=0; i<180; i++)
      delayTask(1L);
   printk("done\r\n");
   printk("timerTests: stopping profiler\r\n");
   disableProf();
   printk("timerTests: Current profiling data follows (Offset:Count):\r\n");
   do {
      profMax = 0;
      for(profIdx=0; profIdx<(sizeof(profBuf)/sizeof(WORD)); profIdx++)
         if(profBuf[profIdx] > profMax)
            profMax = profBuf[profIdx], profMaxIdx = profIdx;
      if(profMax)
      {
         printk("0x%x:%i ", profMaxIdx, profMax);
         profBuf[profMaxIdx] = 0;
      }
   } while(profMax);
   printk("\r\n");

/* Test 5: Remove hooks */
   printk("timerTests: un-trapping BIOS clock\r\n");
   setVector(0x1C, oldBClk);
   printk("timerTests: deleting local timer\r\n");
   if(deleteTimer(myTim))
      return 1;

   printk("timerTests: Done\r\n");
   return 0;
}

static int singleSteps=0;
static BYTE oldOpCode=0;

void far debugTrap(int code, debugRegs_t far *pregs)
{
BYTE far *pOpCode;

/* Don't waste time printing stuff if interrupts are disabled */
   if(pregs->FLAGS & FL_IF)
   {
      printk("debugTrap: code=%i\r\n", code);
      printk("debugTrap: AX=%x BX=%x CX=%x DX=%x SI=%x DI=%x BP=%x  ",
         pregs->AX,
         pregs->BX,
         pregs->CX,
         pregs->DX,
         pregs->SI,
         pregs->DI,
         pregs->BP);
      printk("FLAGS=%c%c%c%c%c%c%c%c%c\r\n",
         (pregs->FLAGS & FL_OF) ? 'O' : 'o',
         (pregs->FLAGS & FL_DF) ? 'D' : 'd',
         (pregs->FLAGS & FL_IF) ? 'I' : 'i',
         (pregs->FLAGS & FL_TF) ? 'T' : 't',
         (pregs->FLAGS & FL_SF) ? 'S' : 's',
         (pregs->FLAGS & FL_ZF) ? 'Z' : 'z',
         (pregs->FLAGS & FL_AF) ? 'A' : 'a',
         (pregs->FLAGS & FL_PF) ? 'P' : 'p',
         (pregs->FLAGS & FL_CF) ? 'C' : 'c');
      FP_SEG(pOpCode) = pregs->CS;
      FP_OFF(pOpCode) = pregs->IP;
      printk("debugTrap: SS:SP=%x:%x DS=%x ES=%x CS:IP=%x:%x (%x)\r\n",
         FP_SEG(pregs),
         FP_OFF(pregs)+sizeof(debugRegs_t),
         pregs->DS,
         pregs->ES,
         pregs->CS,
         pregs->IP,
         *pOpCode);
   }
   switch(code)
   {
   case 1:
   /* Single step, just count them */
      singleSteps++;
      break;
   case 3:
   /* Break point, decrement IP and put oldOpCode back in */
      pregs->IP--;
      FP_OFF(pOpCode) = pregs->IP;
      *pOpCode = oldOpCode;
      break;
   case 0:
   case 2:
   case 4:
      taskExit(-1);
   default:
      quitScheduler(-1);
   }
}

void far breakTrap(void)
{
   taskExit(0x1B);
}

int far debugTask(int arg)
{
union REGS regs;
BYTE far *pOpCode;

   switch(arg)
   {
   case 0:
      arg = 1/arg;      /* Create a divide by zero error - Horrid isn't it? */
      break;
   case 1:                     /* Single stepping so just run to completion */
      break;
   case 3:                         /* Poke a breakpoint into the code, YUK! */
      pOpCode = (BYTE far *)debugTask;
      oldOpCode = *pOpCode;
      *pOpCode = BREAKPOINT;
      debugTask(1);
      break;
   case 2:                       /* NMI & INTO are tricky, so simulate them */
   case 4:
      int86(arg, &regs);
      break;
   case 0x1B:
      printk("debugTask: press Ctrl-Break!\r\n");
      for(;;);
      break;
   default:
      printk("debugTask: unknown test\r\n");
   }
   return arg;
}

void runTest(int code)
{
taskHandle testHandle;
debugRegs_t far *pregs;

   testHandle = newTask(MINSTACK, debugTask, sizeof(int), code);
   if(1 == code)
   {
      pregs = (debugRegs_t far *)testHandle->context;
      pregs->FLAGS |= FL_TF;
   }
   yieldTask();
   printk("trapTests: exit status=%i\r\n", waitTask(testHandle, NOTIMEOUT));
   if(1 == code)
      printk("trapTests: %i single steps\r\n", singleSteps);
}

int trapTests(void)
{
   printk("trapTests: setting debugTrap pointer\r\n");
   setDebuggerTrap(debugTrap);
   printk("trapTests: testing divide-by-zero..");
   runTest(0);
   printk("trapTests: testing single-step..\r\n");
   runTest(1);
   printk("trapTests: testing NMI..");
   runTest(2);
   printk("trapTests: testing breakpoint..");
   runTest(3);
   printk("trapTests: testing overflow..");
   runTest(4);
   printk("trapTests: clearing debugTrap pointer\r\n");
   setDebuggerTrap(NULL);
   printk("trapTests: setting breakTrap pointer\r\n");
   setBreakKeyTrap(breakTrap);
   printk("trapTests: testing Ctrl-Break..");
   runTest(0x1B);
   printk("trapTests: clearing breakTrap pointer\r\n");
   setBreakKeyTrap(NULL);
   return 0;
}

int biosTests(void)
{
int kf;
char buf[40];

   biosCh('H');
   biosCh('e');
   biosCh('l');
   biosCh('l');
   biosCh('o');
   biosCh(' ');
   biosStr("World!\r\n");
   printk("biosTests: peeking for keyboard press\r\n");
   while(!biosKey(BK_PEEK));
   printk("biosTests: reading key press\r\n");
   biosCh(biosKey(BK_READ));
   printk("biosTests: reading keyboard flags, press CapsLock to exit..");
   for(;;)
   {
      kf = biosKey(BK_FLAGS);
      if(kf & BKF_CAPS)
         break;
      sprintf(buf, "Key flags: %x  ", kf);
      outStrXY(60,2,buf);
   }
   printk("biosTests: Done\r\n");
   return 0;
}


/* Define the size of mainTask()'s stack here */
int mainStackSize = MINSTACK;

/* Entry point to user program, parses command line args for test numbers
   and runs those tests */

void far mainTask(char far *cmdLine)
{
char c, far *opArg, far *msgBuf = (char far *)MSG_ADDR;
int speed;

/* Display copyright message and version info */
   printk("Kerntest.c Copyright (c) (AshbySoft *) 1995: "
          "$Revision: 1.0(Alpha) $\r\n\r\n");

/* Configure message buffer */
   memset(msgBuf, 0, MSG_SIZE);
   setPrintk(NULL, msgBuf, MSG_SIZE);

/* Parse command line arguments & run specified tests */
   while((c = getopts(cmdLine, "m12:34p", &opArg)) > 0)
   {
      switch(c)
      {
      case 'm':
         screen = (int far *)MDA_ADDR;
         break;
      case '1':
         if(taskTests())
            return;
         break;
      case '2':
         if(*opArg == 's')
            speed = 100;
         else
            speed = 1000;
         if(timerTests(speed))
            return;
         break;
      case '3':
         if(trapTests())
            return;
         break;
      case '4':
         if(biosTests())
            return;
         break;
      case 'p':
         panic("testing panic..");
         break;
      }
   }
   printk("mainTask(): All tests completed OK\r\n");
}

