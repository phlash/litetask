#include <litetask.h>

int mainStackSize = MINSTACK;

void far mainTask(char far *cmd)
{
int fsId;

   installTimer(1193);
   if(startProfiling())
      return;
   initFileSystem();
   initFatFs();
   fsId = newFatFs(2, 0L, "c:");
   if(fsId < 0)
      return;
   printk("mainTask(): delaying for 10 seconds..");
   delayTask(10000L);
   printk("Done\r\n");
   if(stopProfiling("c:proftest.prf"))
      return;
   deleteFatFs(fsId);
   removeFatFs();
   removeFileSystem();
}

