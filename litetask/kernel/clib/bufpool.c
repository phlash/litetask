/*------------------------------------------------------------------------
   BUFPOOL.C - LiteTask buffer pool functions

   $Author:   Phlash  $
   $Date:   26 Mar 1995 10:20:08  $
   $Revision:   1.0  $

------------------------------------------------------------------------*/
#include "litetask.h"
#include "debug.h"

bufPool_t far * far createBufPool(unsigned short blocks, unsigned short size)
{
long memSize;
bufPool_t far *newPool;
buf_t far *pBuf;
unsigned short cnt;

/* sanity check */
   if(!blocks || !size)
   {
      LT_DBG(DBG_KERN_ERROR, "createBufPool: pool size=0\r\n");
      return NULL;
   }

/* malloc buffer area */
   memSize = (long)sizeof(bufPool_t) + (long)blocks *
             ((long)size + (long)sizeof(buf_t) - 1L);
   if(memSize > 0xFFF0)
   {
      LT_DBG(DBG_KERN_ERROR, "createBufPool: pool size too big\r\n");
      return NULL;
   }

   newPool = (bufPool_t far *)malloc((unsigned short)memSize);
   if(!newPool)
      return NULL;

/* create buffers */
   newPool->free = blocks;
   newPool->head = (buf_t far *)((char far *)newPool + sizeof(bufPool_t));
   for(cnt=0, pBuf = newPool->head; cnt<blocks;
       cnt++, newPool->tail = pBuf, pBuf = pBuf->next)
      pBuf->next = (cnt == blocks) ? NULL : 
         (buf_t far *)((char far *)pBuf + sizeof(buf_t) - 1 + size);
   return newPool;
}


int far deleteBufPool(bufPool_t far *oldPool)
{
   if(!oldPool)
      return -1;
   return free(oldPool);
}

int far resizeBufPool(bufPool_t far *bufPool, unsigned short blocks)
{
/* XXXX: I'lll do this later :) */
   return -1;
}

buf_t far * far getBuffer(bufPool_t far *bufPool)
{
short flag;
buf_t far *rBuf;

   if(!bufPool)
      return NULL;

   flag = lockInts();
   if(!bufPool->free)
   {
      unlockInts(flag);
      return NULL;
   }
   if(!--bufPool->free)
      bufPool->tail = NULL;
   rBuf = bufPool->head;
   bufPool->head = bufPool->head->next;
   unlockInts(flag);
   return rBuf;
}

int far putBuffer(bufPool_t far *bufPool, buf_t far *oBuf)
{
short flag;

   if(!bufPool || !oBuf)
      return -1;
   flag = lockInts();
   bufPool->free++;
   if(bufPool->tail)
      bufPool->tail->next = oBuf;
   else
      bufPool->head = oBuf;
   bufPool->tail = oBuf;
   unlockInts(flag);
   return 0;
}

