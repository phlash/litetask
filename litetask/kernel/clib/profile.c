/*------------------------------------------------------------------------
   PROFILE.C - LiteTask profiler library

   $Author:$
   $Date:$
   $Revision:$

------------------------------------------------------------------------*/
#include "litetask.h"
#include "debug.h"

static WORD far *profBuf = NULL;
static WORD profBufLen = 0;

/*
 * Start profiling
 */
int far startProfiling(void)
{
char far *startAddr, far *endAddr;
short flag = lockTask();

   if(profBuf)
   {
      unlockTask(flag);
      LT_DBG(DBG_CLIB_ERROR, "startProfiling(): Already doing it\r\n");
      return -1;
   }
   startAddr = (char far *)startProfiling;
   FP_OFF(startAddr) = 0;
   endAddr = (char far *)&mainStackSize;
   profBufLen = (FP_SEG(endAddr) - FP_SEG(startAddr)) << 4;
   profBuf = (WORD far *)malloc(sizeof(WORD)*profBufLen);
   if(!profBuf)
   {
      unlockTask(flag);
      LT_DBG(DBG_CLIB_ERROR, "startProfiling(): Cannot allocate buffer\r\n");
      return -1;
   }
   memset(profBuf, 0, sizeof(WORD)*profBufLen);
   if(enableProf(startAddr, profBuf, profBufLen))
   {
      LT_DBG(DBG_CLIB_ERROR, "startProfiling(): Error enabling profiler\r\n");
      free(profBuf);
      profBuf = NULL;
      unlockTask(flag);
      return -1;
   }
   unlockTask(flag);
   return 0;
}

int far stopProfiling(char far *saveFile)
{
WORD lPLen, far *lPBuf;
int fd, tmp;
short flag = lockTask();

   if(!profBuf)
   {
      unlockTask(flag);
      LT_DBG(DBG_CLIB_ERROR, "stopProfiling(): Not doing any\r\n");
      return -1;
   }
   disableProf();
   lPBuf = profBuf;
   lPLen = profBufLen;
   profBuf = NULL;
   unlockTask(flag);
   if(!saveFile)
      saveFile = "c:prof.dat";
   fd = open(saveFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
   if(fd < 0)
   {
      LT_DBG(DBG_CLIB_ERROR, "stopProfiling(): Cannot create save file\r\n");
      return -1;
   }
   tmp = getLockOuts();
   write(fd, (char far *)&tmp, sizeof(tmp));
   tmp = getTaskSwitches();
   write(fd, (char far *)&tmp, sizeof(tmp));
   tmp = getIdleSwitches();
   write(fd, (char far *)&tmp, sizeof(tmp));
   tmp = getScheduleTime();
   write(fd, (char far *)&tmp, sizeof(tmp));
   write(fd, (char far *)lPBuf, sizeof(WORD)*lPLen);
   close(fd);
   free(lPBuf);
   return 0;
}

/* End */
