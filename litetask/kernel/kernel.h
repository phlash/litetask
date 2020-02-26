/*------------------------------------------------------------------------
   KERNEL.H - LiteTask Kernel internal functions

   $Author:   Phlash  $
   $Date:   24 Jun 1995 20:23:46  $
   $Revision:   1.0  $

------------------------------------------------------------------------*/

/* MAIN.C */
extern void far main(char far *commandTail);
extern void far panic(char far *msg);

/* TASK.C */
extern int far startScheduler(char far *commandTail);

/* IDLE.C */
extern void far idleTaskFun(void);

/* TRAPS.C */
extern void far setCPUTraps(void);
extern void far clearCPUTraps(void);
extern void far setBIOSTraps(void);
extern void far clearBIOSTraps(void);

/* Lethal macros */
#define CHKSIG(test, sig) ( *(long far *)(test) != *(long far *)(sig) )

/* End */
