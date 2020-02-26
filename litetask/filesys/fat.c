/*------------------------------------------------------------------------
   FAT.C - LiteTask FAT (MS-DOS) file system driver

   $Author:   Phlash  $
   $Date:   10 Feb 1995 22:51:58  $
   $Revision:   1.1  $

------------------------------------------------------------------------*/

#include "litetask.h"
#include "filesys.h"
#include "fat.h"

/*------------------------------------------------------------------------
   NOTES:
   1/ Currently uses DOS as the underlying driver, not the internal disk
      driver(s)...
   2/ As a result, the device parameter to newFatFs() is the drive number.
------------------------------------------------------------------------*/

/* local data */
static union REGS regs;
static void far *oldInt24;

/* Constants */
#define DOS_INT   0x21

/* prototypes */
static void far * far fatOpen(void far *fsInfo, char far *path, int mode, int perm);
static int far fatClose(void far *fsInfo, void far *fInfo);
static int far fatRead(void far *fsInfo, void far *fInfo, char far *buf, int len);
static int far fatWrite(void far *fsInfo, void far *fInfo, char far *buf, int len);
static int far fatLseek(void far *fsInfo, void far *fInfo, long offset, int mode);
static int far fatIoctl(void far *fsInfo, void far *fInfo, int ioctl, void far *arg);

/* user entry points */
int far initFatFs(void)
{
/* Trap DOS crtical errors */
   oldInt24 = setVector(0x24, int24Trap);
   return 0;
}

int far newFatFs(int device, long offset, char far *name)
{
filesystem_t far *pfs;
short flag;
int fsId;

/* check that this drive exists (and it has a disk in :) */
   flag = lockTask();
   regs.x.ax = 0x1C00;
   regs.h.dl = (BYTE)device+1;
   int86(DOS_INT, &regs);
   if(regs.h.al == 0xFF)
   {
      unlockTask(flag);
      printk("FAT FS: Invalid MS-DOS drive number\r\n");
      return -1;
   }
   unlockTask(flag);

/* allocate file system info */
   pfs = (filesystem_t far *)malloc( sizeof(filesystem_t) );
   if(!pfs)
      return -1;
   strcpy(pfs->name, name);
   pfs->useCnt= 0;
   FP_SEG(pfs->fsInfo) = 0;
   FP_OFF(pfs->fsInfo) = device;
   pfs->open  = fatOpen;
   pfs->close = fatClose;
   pfs->read  = fatRead;
   pfs->write = fatWrite;
   pfs->lseek = fatLseek;
   pfs->ioctl = fatIoctl;
   if((fsId=newFileSystem(pfs)) < 0)
   {
      free(pfs);
      printk("FAT FS: Unable to install in file system table\r\n");
      return -1;
   }
   return fsId;
}

int far deleteFatFs(int fsId)
{
   return deleteFileSystem(fsId);
}

int far removeFatFs(void)
{
   setVector(0x24, oldInt24);
   return 0;
}

static void far * far fatOpen(void far *fsInfo, char far *path, int mode, int perm)
{
char drPath[128], far *pPath = drPath;
void far *handle;
short flag;

/* Create drive/path name */
   sprintf(drPath, "%c:%s", FP_OFF(fsInfo) + 'A', path);
#ifdef DEBUG
   printk("FAT FS: opening: ");
   printk(drPath);
   printk("\r\n");
#endif

/* Open via DOS */
   flag = lockTask();
   regs.h.ah = 0x3D;
   regs.h.al = (BYTE)mode & (O_RDONLY|O_WRONLY|O_RDWR);
   regs.x.ds = FP_SEG(pPath);
   regs.x.dx = FP_OFF(pPath);
   if(int86(DOS_INT, &regs))
   {
      if(mode & O_CREAT)
      {
         regs.h.ah = 0x3C;
         regs.h.al = 0;
         regs.x.cx = (perm & 0400) ? 0 : 1;
         regs.x.ds = FP_SEG(pPath);
         regs.x.dx = FP_OFF(pPath);
         if(int86(DOS_INT, &regs))
         {
            unlockTask(flag);
            return NULL;
         }
      }
      else
      {
         unlockTask(flag);
         return NULL;
      }
   }
   FP_OFF(handle) = regs.x.ax;
   FP_SEG(handle) = 0;
   if(mode & O_TRUNC)
   {
   /* Truncate by writing zero bytes at current location */
      regs.x.bx = regs.x.ax;
      regs.h.ah = 0x40;
      regs.x.cx = 0;
      int86(DOS_INT, &regs);
   }
   unlockTask(flag);
   return handle;
}

static int far fatClose(void far *fsInfo, void far *fInfo)
{
short flag;

   flag = lockTask();
   regs.h.ah = 0x3E;
   regs.x.bx = FP_OFF(fInfo);
   if(int86(DOS_INT, &regs))
   {
      unlockTask(flag);
      return -1;
   }
   unlockTask(flag);
   return 0;
}

static int far fatRead(void far *fsInfo, void far *fInfo, char far *buf, int len)
{
short flag;

   flag = lockTask();
   regs.h.ah = 0x3F;
   regs.x.bx = FP_OFF(fInfo);
   regs.x.cx = len;
   regs.x.ds = FP_SEG(buf);
   regs.x.dx = FP_OFF(buf);
   if(int86(DOS_INT, &regs))
   {
      unlockTask(flag);
      return -1;
   }
   unlockTask(flag);
   return regs.x.ax;
}

static int far fatWrite(void far *fsInfo, void far *fInfo, char far *buf, int len)
{
short flag;

   flag = lockTask();
   regs.h.ah = 0x40;
   regs.x.bx = FP_OFF(fInfo);
   regs.x.cx = len;
   regs.x.ds = FP_SEG(buf);
   regs.x.dx = FP_OFF(buf);
   if(int86(DOS_INT, &regs))
   {
      unlockTask(flag);
      return -1;
   }
   unlockTask(flag);
   return regs.x.ax;
}

static int far fatLseek(void far *fsInfo, void far *fInfo, long offset, int mode)
{
short flag;

   flag = lockTask();
   regs.h.ah = 0x42;
   regs.h.al = (BYTE)mode;
   regs.x.bx = FP_OFF(fInfo);
   regs.x.cx = (WORD)(offset >> 8);
   regs.x.dx = (WORD)offset;
   if(int86(DOS_INT, &regs))
   {
      unlockTask(flag);
      return -1;
   }
   unlockTask(flag);
   return 0;
}

static int far fatIoctl(void far *fsInfo, void far *fInfo, int ioctl, void far *arg)
{
short flag;

/* Currently supports get/set device information via DX register */
   flag = lockTask();
   regs.h.ah = 0x44;
   regs.h.al = (BYTE)ioctl;
   regs.x.bx = FP_OFF(fInfo);
   regs.x.dx = *((int far *)arg);
   if(int86(DOS_INT, &regs))
   {
      unlockTask(flag);
      return -1;
   }
   *((int far *)arg) = regs.x.dx;
   unlockTask(flag);
   return 0;
}

/* End */
