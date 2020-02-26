/*------------------------------------------------------------------------
   DEBUG.C - Debugger for LiteTask Kernel

   $Author:   Phlash  $
   $Date:   24 Jun 1995 20:22:12  $
   $Revision:   1.0  $

------------------------------------------------------------------------*/

#include "litetask.h"
#include "debug.h"

/* Global debug function pointer */
void (far *debugFunPtr)(char far *) = printk;

/* Default debug flags */
int debugFlags = DBG_SRC_LTASK | DBG_TYPE_ERROR;

/* Change the global pointer without breaking things */
void far setDebug(void (far *newDebugPtr)(char far *))
{
short flag;

   flag = lockInts();
   if(newDebugPtr)
   {
      debugFunPtr = newDebugPtr;
   }
   else
   {
      debugFunPtr = printk;
   }
   unlockInts(flag);
}

/* Pointer formatting routine */
char far * far formatHex(void far *ptr)
{
static char buf[12];

   sprintf(buf, "%x:%x\r\n", FP_SEG(ptr), FP_OFF(ptr));
   return buf;
}

/* End */

