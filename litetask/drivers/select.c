/*------------------------------------------------------------------------
   SELECT.C - LiteTask device selection function

   $Author:   Phlash  $
   $Date:   03 Jun 1994 19:53:40  $
   $Revision:   1.3  $

------------------------------------------------------------------------*/

#include "litetask.h"
#include "select.h"
#include "ioctl.h"

int far select(int type, int nDevs, ...)
{
int far *pDev;
selectInfo_t far *selects;
int ioc, sDevs, cDev, rDev;
short flag;
long bytes;
char msg[60];

/* sanity checks */
   if(nDevs < 1)
      return EARGS;
   if(type != SEL_READ && type != SEL_WRITE)
      return EARGS;

/* allocate select data */
   selects = (selectInfo_t far *)malloc( nDevs * sizeof(selectInfo_t) );
   if(!selects)
   {
      printk("select: no memory available\r\n");
      return EINTERNAL;
   }

/* lock interrupts during this operation */
   flag = lockInts();

/* check if any devices are ready to go now */
   pDev = &nDevs;
   pDev++;
   if(type == SEL_READ)
      ioc = STDIOCNREAD;
   else
      ioc = STDIOCNWRITE;
   for(cDev=0; cDev<nDevs; cDev++)
   {
      bytes = 0L;
      if(!ioctlDevice(*pDev, ioc, &bytes))
      {
         if(bytes)
         {
            unlockInts(flag);
            free(selects);
            return *pDev;
         }
      }
      pDev++;
   }

/* set up select notification on all requested devices */
   pDev = &nDevs;
   pDev++;
   sDevs = 0;
   for(cDev=0; cDev<nDevs; cDev++)
   {
      selects[cDev].type = type;
      selects[cDev].task = getTaskHandle();
      if(ioctlDevice(*pDev, STDIOCSELECT, &selects[cDev]) < 0)
      {
         selects[cDev].task = NULL;
         sprintf(msg, "select: error selecting on device %i\r\n", *pDev);
         printk(msg);
      }
      else
         sDevs++;
      pDev++;
   }

/* now sleep on it! (unless nothing could be selected) */
   if(sDevs)
      rDev = suspendTask();
   else
   {
      unlockInts(flag);
      free(selects);
      return EBADDEV;
   }

/* clear select notification */
   lockInts();
   pDev = &nDevs;
   pDev++;
   for(cDev=0; cDev<nDevs; cDev++)
   {
      if(selects[cDev].task)
      {
         if(ioctlDevice(*pDev, STDIOCNSELECT, &selects[cDev]) < 0)
         {
            sprintf(msg, "select: error unselecting on device %i\r\n", *pDev);
            printk(msg);
         }
      }
      pDev++;
   }

/* unlock interrupts */
   unlockInts(flag);

/* free memory and return selected device */
   free(selects);
   return rDev;
}

/* End */
