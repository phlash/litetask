/*------------------------------------------------------------------------
   TRAPS.C - LiteTask CPU / BIOS trap handlers

   $Author:   Phlash  $
   $Date:   24 Jun 1995 20:24:52  $
   $Revision:   1.0  $

------------------------------------------------------------------------*/
#include "litetask.h"
#include "debug.h"
#include "kernel.h"

/*
 * Local prototypes
 */
void far divZero(debugRegs_t);
void far oneStep(debugRegs_t);
void far nmiInt(debugRegs_t);
void far breakPoint(debugRegs_t);
void far overFlow(debugRegs_t);

void far breakKey(void);

/*
 * External definitions (in TRAPGLUE.ASM)
 */
extern void far divZeroTrap(void);
extern void far oneStepTrap(void);
extern void far nmiIntTrap(void);
extern void far breakPointTrap(void);
extern void far overFlowTrap(void);
extern void far breakKeyTrap(void);

/*
 * Globals
 */
/* CPU traps */
#define DIVZERO   0
#define ONESTEP   1
#define NMIINT    2
#define BREAKP    3
#define OVERFLOW  4

static void far *oldDivZero;
static void far *oldOneStep;
static void far *oldNmiInt;
static void far *oldBreakPoint;
static void far *oldOverflow;

static void (far *debuggerTrap)(int, debugRegs_t far *) = NULL;

/* BIOS traps */
#define KEYBREAK  0x1B

static void far *oldBreakKey;

static void (far *breakTrap)(void) = NULL;

/*
 * Install CPU traps
 */
void far setCPUTraps(void)
{
short flag;

   flag = lockInts();
   oldDivZero = setVector(DIVZERO, divZeroTrap);
   oldOneStep = setVector(ONESTEP, oneStepTrap);
   oldNmiInt = setVector(NMIINT, nmiIntTrap);
   oldBreakPoint = setVector(BREAKP, breakPointTrap);
   oldOverflow = setVector(OVERFLOW, overFlowTrap);
   unlockInts(flag);
}

void far clearCPUTraps(void)
{
short flag;

   flag = lockInts();
   setVector(DIVZERO, oldDivZero);
   setVector(ONESTEP, oldOneStep);
   setVector(NMIINT, oldNmiInt);
   setVector(BREAKP, oldBreakPoint);
   setVector(OVERFLOW, oldOverflow);
   unlockInts(flag);
}

/*
 * Modify debugger trap
 */
void far setDebuggerTrap(void (far *newTrap)(int, debugRegs_t far *))
{
   debuggerTrap = newTrap;
}

/*
 * traps for CPU errors
 */
void far divZero(debugRegs_t regs)
{
/* if we have a debugger, call it */
   if(debuggerTrap)
      debuggerTrap(DIVZERO, &regs);
   else
   {
   /* kill this task */
      LT_DBG(DBG_KERN_ERROR, "divZero(): Divide by zero in task=");
      LT_DBG(DBG_KERN_ERROR, formatHex(getTaskHandle()));
      taskExit(-1);
   }
}

void far oneStep(debugRegs_t regs)
{
/* if we have a debugger, then call it, otherwise kill task */
   if(debuggerTrap)
      debuggerTrap(ONESTEP, &regs);
   else
   {
      LT_DBG(DBG_KERN_ERROR, "oneStep(): Single step in task=");
      LT_DBG(DBG_KERN_ERROR, formatHex(getTaskHandle()));
      taskExit(-1);
   }
}

void far nmiInt(debugRegs_t regs)
{
/* You never, know, a debugger could do something.. */
   if(debuggerTrap)
      debuggerTrap(NMIINT, &regs);
   else
   {
   /* ARRGH! usually a memory parity error, we die. */
      LT_DBG(DBG_KERN_ERROR, "nmiTrap(): NMI in task=");
      LT_DBG(DBG_KERN_ERROR, formatHex(getTaskHandle()));
      panic("***** NMI interrupt occurred *****");
   }
}

void far breakPoint(debugRegs_t regs)
{
/* if we have a debugger, call it */
   if(debuggerTrap)
      debuggerTrap(BREAKP, &regs);
   else
   {
   /* kill offending task */
      LT_DBG(DBG_KERN_ERROR, "breakPoint(): Breakpoint in task=");
      LT_DBG(DBG_KERN_ERROR, formatHex(getTaskHandle()));
      taskExit(-1);
   }
}

void far overFlow(debugRegs_t regs)
{
/* again, if a debugger is present.. */
   if(debuggerTrap)
      debuggerTrap(OVERFLOW, &regs);
   else
   {
   /* kill offending task */
      LT_DBG(DBG_KERN_ERROR, "overflow(): Overflow in task=");
      LT_DBG(DBG_KERN_ERROR, formatHex(getTaskHandle()));
      taskExit(-1);
   }
}


void far setBIOSTraps(void)
{
   oldBreakKey = setVector(KEYBREAK, breakKeyTrap);
}

void far clearBIOSTraps(void)
{
   setVector(KEYBREAK, oldBreakKey);
}

void (far * far setBreakKeyTrap(void (far *newTrap)(void)) )(void)
{
void (far *oldBreak)(void);

   oldBreak = breakTrap;
   breakTrap = newTrap;
   return oldBreak;
}

void far breakKey(void)
{
/* If a break key trap is installed, run it. Otherwise we die. */
   if(breakTrap)
      breakTrap();
   else
   {
      LT_DBG(DBG_KERN_ERROR, "breakKey(): break key pressed in task=");
      LT_DBG(DBG_KERN_ERROR, formatHex(getTaskHandle()));
      quitScheduler(1);
   }
}

/* End */
