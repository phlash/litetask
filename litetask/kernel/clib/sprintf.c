/*-------------------------------------------------------------------------
   SPRINTF.C - Simple text string formatting

   $Author:   Phlash  $
   $Date:   16 Sep 1995 17:19:18  $
   $Revision:   1.7  $

-------------------------------------------------------------------------*/

#include "litetask.h"
#include "doprint.h"

/* Add a character to the output buffer, increment buffer pointer */

static void far addToBuffer(char far * far *pBuf, int ch)
{
   **pBuf = (char)ch;
   (*pBuf)++;
}

/* Format text into supplied buffer */

int far sprintf(char far *buf, char far *fmt, ...)
{
char far *argp;
int cnt;

/* sanity checks */
   if(buf == NULL || fmt == NULL)
      return 0;

/* get first argument address */
   argp = ((char far *)&fmt) + sizeof(fmt);

/* process format string */
   cnt = doPrint(addToBuffer, &buf, fmt, argp);
   *buf = 0;
   cnt++;
   return cnt;
}

/* End */
