/*------------------------------------------------------------------------
   FILESYS.C - LiteTask generic file system

   $Author:   Phlash  $
   $Date:   10 Feb 1995 22:51:36  $
   $Revision:   1.1  $

------------------------------------------------------------------------*/

#include "litetask.h"
#include "filesys.h"

/* The file system table */
static filesystem_t fsTab[MAX_FILE_SYSTEMS];

/* The system file handle table */
typedef struct fhtab_tag {
                  int openCnt;
                  int fsIdx;
                  void far *fInfo;
                  } fhtab_t;
static fhtab_t fhTab[MAX_FILES];

/* access semaphores for these tables */
static semaphore_t fsTabSem, fhTabSem;

/*------------------------------------------------------------------------
NOTES:
1/ Client routines maintain a useCnt in each file system table entry for
pending file operations. This allows the deleteFileSys() routine to busy
wait to ensure that no operations are in progress on a file system when
deleting it. However, this may busy wait forever or deadlock the application
if the file system is still in use. BEWARE!

------------------------------------------------------------------------*/

/* Client routines */
int far open(char far *path, int mode, int perm)
{
int fs,len,fd;
void far *fi;

/* search fsTab for matching root path name */
   getSemaphore(&fsTabSem, NOTIMEOUT);
   for(fs=0; fs<MAX_FILE_SYSTEMS; fs++)
   {
      if(fsTab[fs].name[0])
      {
         len=strlen(fsTab[fs].name);
         if(!memcmp(fsTab[fs].name, path, len))
            break;
      }
   }
   if(fs >= MAX_FILE_SYSTEMS)
   {
      putSemaphore(&fsTabSem);
      return -1;
   }
   fsTab[fs].useCnt++;
   putSemaphore(&fsTabSem);

/* Call driver open routine */
   fi = fsTab[fs].open(fsTab[fs].fsInfo, &path[len], mode, perm);
   if(!fi)
   {
      fsTab[fs].useCnt--;
      return -1;
   }

/* allocate a new file handle */
   getSemaphore(&fhTabSem, NOTIMEOUT);
   for(fd=0; fd<MAX_FILES; fd++)
   {
      if(!fhTab[fd].openCnt)
         break;
   }
   if(fd >= MAX_FILES)
   {
      putSemaphore(&fhTabSem);
      fsTab[fs].close(fsTab[fs].fsInfo, fi);
      fsTab[fs].useCnt--;
      printk("open(): No file handles left\r\n");
      return -1;
   }
   fhTab[fd].openCnt = 1;
   fhTab[fd].fsIdx   = fs;
   fhTab[fd].fInfo   = fi;
   putSemaphore(&fhTabSem);
   return fd;
}

int far close(int fd)
{
int cnt, id, rv;
void far *fi;

/* Sanity check */
   if(!fhTab[fd].openCnt)
      return -1;

/* decrement handle table open count (may free entry in table) */
   getSemaphore(&fhTabSem, NOTIMEOUT);
   id = fhTab[fd].fsIdx;
   fi = fhTab[fd].fInfo;
   cnt = --fhTab[fd].openCnt;
   putSemaphore(&fhTabSem);

/* internal sanity checks */
   if(fsTab[id].useCnt <= 0)
   {
      printk("close(): Invalid file system use count detected\r\n");
      return -1;
   }

/* call driver close routine on last closure of this file */
   if(!cnt)
   {
      rv = fsTab[id].close(fsTab[id].fsInfo, fi);
      fsTab[id].useCnt--;
   }
   else
      rv = 0;
   return rv;
}

int far read(int fd, char far *buf, int len)
{
int rv, id;
void far *fi;

/* Sanity check */
   if(!fhTab[fd].openCnt)
      return -1;

/* Call driver read routine */
   getSemaphore(&fhTabSem, NOTIMEOUT);
   id = fhTab[fd].fsIdx;
   fi = fhTab[fd].fInfo;
   putSemaphore(&fhTabSem);
   rv = fsTab[id].read(fsTab[id].fsInfo, fi, buf, len);
   return rv;
}

int far write(int fd, char far *buf, int len)
{
int rv, id;
void far *fi;

/* Sanity check */
   if(!fhTab[fd].openCnt)
      return -1;

/* Call driver write routine */
   getSemaphore(&fhTabSem, NOTIMEOUT);
   id = fhTab[fd].fsIdx;
   fi = fhTab[fd].fInfo;
   putSemaphore(&fhTabSem);
   rv = fsTab[id].write(fsTab[id].fsInfo, fi, buf, len);
   return rv;
}

int far lseek(int fd, long offset, int location)
{
int rv, id;
void far *fi;

/* Sanity check */
   if(!fhTab[fd].openCnt)
      return -1;

/* Call driver lseek routine */
   getSemaphore(&fhTabSem, NOTIMEOUT);
   id = fhTab[fd].fsIdx;
   fi = fhTab[fd].fInfo;
   putSemaphore(&fhTabSem);
   rv = fsTab[id].lseek(fsTab[id].fsInfo, fi, offset, location);
   return rv;
}

int far ioctl(int fd, int op, void far *arg)
{
int rv, id;
void far *fi;

/* Sanity check */
   if(!fhTab[fd].openCnt)
      return -1;

/* Call driver ioctl routine */
   getSemaphore(&fhTabSem, NOTIMEOUT);
   id = fhTab[fd].fsIdx;
   fi = fhTab[fd].fInfo;
   putSemaphore(&fhTabSem);
   rv = fsTab[id].ioctl(fsTab[id].fsInfo, fi, op, arg);
   return rv;
}

/* File system routines */

int far initFileSystem(void)
{
int i;

/* set up semaphores & fsTab / fhTab correctly */
   initSemaphore(&fsTabSem);
   initSemaphore(&fhTabSem);
   getSemaphore(&fsTabSem, NOTIMEOUT);
   for(i=0; i<MAX_FILE_SYSTEMS; i++)
   {
      fsTab[i].name[0]= 0;
      fsTab[i].useCnt = 0;
      fsTab[i].fsInfo = NULL;
      fsTab[i].open   = NULL;
      fsTab[i].close  = NULL;
      fsTab[i].read   = NULL;
      fsTab[i].write  = NULL;
      fsTab[i].lseek  = NULL;
   }
   putSemaphore(&fsTabSem);
   getSemaphore(&fhTabSem, NOTIMEOUT);
   for(i=0; i<MAX_FILES; i++)
   {
      fhTab[i].openCnt = 0;
      fhTab[i].fsIdx   = 0;
      fhTab[i].fInfo   = NULL;
   }
   putSemaphore(&fhTabSem);
   return 0;
}

int far removeFileSystem(void)
{
/* May do something later.. */
   return 0;
}

int far newFileSystem(filesystem_t far *newFs)
{
int newIdx;

/* Search fsTab for free entry */
   getSemaphore(&fsTabSem, NOTIMEOUT);
   for(newIdx = 0; newIdx < MAX_FILE_SYSTEMS; newIdx++)
   {
      if(fsTab[newIdx].name[0] == 0)
         break;
   }
   if(newIdx >= MAX_FILE_SYSTEMS)
   {
      putSemaphore(&fsTabSem);
      return EINTERNAL;
   }
   fsTab[newIdx] = *newFs;
   fsTab[newIdx].useCnt = 0;
   putSemaphore(&fsTabSem);
   return newIdx;
}

int far deleteFileSystem(int fsIndex)
{
long t;

/* sanity check */
   if(!fsTab[fsIndex].name[0])
      return EINTERNAL;

/* busy wait until file system is not in use */
   t = getTicks();
   for(;;)
   {
      getSemaphore(&fsTabSem, NOTIMEOUT);
      if(!fsTab[fsIndex].useCnt)
         break;
      putSemaphore(&fsTabSem);
      if(getTicks() > (t + BUSY_WAIT_TIME))
      {
         printk("deleteFileSystem(");
         printk(fsTab[fsIndex].name);
         printk("): Still waiting for useCnt=0\r\n");
         t = getTicks();
      }
      yieldTask();
   }

/* mark entry in fsTab as free */
   fsTab[fsIndex].name[0] = 0;
   putSemaphore(&fsTabSem);
   return 0;
}

/* End */
