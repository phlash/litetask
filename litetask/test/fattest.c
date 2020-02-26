#include "litetask.h"

int mainStackSize = MINSTACK;

void far fileTask(int line, int drv)
{
int fd, len;
char buf[80];

/* say hello */
again:
   sprintf(buf, "Starting drive %c:  ", drv);
   outstring(buf, 0, line);
   outstring("                          ", 0, line+1);
   delayTask(9L);

/* open a file on the system */
   sprintf(buf, "%c:fattest.dat", drv);
   if((fd=open(buf, O_RDWR|O_CREAT|O_TRUNC, 0666)) < 0)
   {
      outstring("Error opening file! ", 0, line+1);
      return;
   }

/* Get device mode */
   if(ioctl(fd, 0, &len) < 0)
   {
      outstring("Error reading mode! ", 0, line+1);
      goto end;
   }
   sprintf(buf, "Device mode: %x     ", len);
   outstring(buf, 0, line);

/* write a few bytes */
   len = sprintf(buf, "Hello World!");
   if(write(fd, buf, len+1) < 0)
   {
      outstring("Error writing file! ", 0, line+1);
      goto end;
   }

/* Seek back to start */
   if(lseek(fd, 0L, SEEK_SET) < 0)
   {
      outstring("Error seeking file! ", 0, line+1);
      goto end;
   }

/* Read bytes back again */
   if((len=read(fd, buf, sizeof(buf))) < 0)
   {
      outstring("Error reading file! ", 0, line+1);
      goto end;
   }
   outstring("Read: ", 0, line);
   outstring(buf, 6, line);

end:
   outstring("15sec delay..       ", 0, line);
   delayTask(270L);
   close(fd);
   goto again;
}

void far mainTask()
{
int fsId[4], fs;
char buf[80];

/* install file system support */
   if(initFileSystem())
      return;

/* install FAT FS */
   if(initFatFs())
      return;

/* create file systems & spawn file tasks */
   for(fs=0; fs<4; fs++)
   {
      outstring("mainTask: creating drives..", 0, 22);
      sprintf(buf, "%c:", fs + 'a');
      if((fsId[fs]=newFatFs(fs, 0L, buf)) < 0)
      {
         printk("Unable to create file system for drive: ");
         printk(buf);
         printk("\r\n");
      }
      else
         newTask(MINSTACK, fileTask, 2*sizeof(int), 3*fs+10, fs + 'a');
   }
   delayTask(180L);

/* now attempt to delete file systems (should block until free) */
   outstring("mainTask: attempting to delete file systems..", 0, 22);
   for(fs=0; fs<4; fs++)
   {
      if(fsId[fs] >= 0)
         deleteFatFs(fsId[fs]);
   }
   outstring("mainTask: done.", 0, 23);
   removeFatFs();
   removeFileSystem();
   return;
}

