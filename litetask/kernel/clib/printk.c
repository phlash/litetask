/*------------------------------------------------------------------------
   PRINTK.C - Kernel string output, supports screen and/or memory buffer

   $Author:   Phlash  $
   $Date:   16 Jul 1995  9:13:32  $
   $Revision:   1.3  $

------------------------------------------------------------------------*/

#include "litetask.h"
#include "doprint.h"

/* pointer to screen output driver (if we are using one) */
static textDriver_t far *pDrv = NULL;

/* pointer & size for memory buffer (if we are using one) */
static char far *pBuf = NULL;
static int bufferSize = 0;
static int bufferLoc = 0;

/* Place characters on screen/in buffer */

void far printkOut(void far *arg, int ch)
{
short flag;

   flag = lockTask();
   if(pDrv)
      pDrv->OutCh(ch);
   else
      biosCh(ch);
   if(pBuf)
   {
      pBuf[bufferLoc++] = (char)ch;
      if(bufferLoc >= bufferSize)
         bufferLoc = 0;
   }
   unlockTask(flag);
}

int far printk(char far *fmt, ...)
{
char far *argp;

/* check args */
   if(fmt == NULL)
      return -1;

/* get address of first argument */
   argp = ((char far *)&fmt) + sizeof(char far *);

/* Format the string */
   return doPrint(printkOut, NULL, fmt, argp);
}

void far setPrintk(textDriver_t far *drv, char far *buf, int size)
{
short flag = lockTask();

/* set up the flags & pointers */
   pDrv = drv;
   pBuf = buf;
   bufferSize = size;
   unlockTask(flag);
}

/* End */
