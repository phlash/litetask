/*------------------------------------------------------------------------
   FILESYS.H - LiteTask generic file system definitions

   $Author:   Phlash  $
   $Date:   10 Feb 1995 22:51:30  $
   $Revision:   1.1  $

------------------------------------------------------------------------*/

#ifndef _FILESYS_H
#define _FILESYS_H

/* Constants */
#define MAX_FILE_SYSTEMS   8
#define MAX_FILES          256
#define BUSY_WAIT_TIME     182

/* Top-level file system structures */

#define MAX_FS_NAME_LEN    20
typedef struct filesystem_tag {
   char name[MAX_FS_NAME_LEN];
   int useCnt;
   void far *fsInfo;
   void far *(far *open)(void far *fsInfo, char far *path, int mode, int perm);
   int (far *close)(void far *fsInfo, void far *fInfo);
   int (far *read) (void far *fsInfo, void far *fInfo, char far *buf, int len);
   int (far *write)(void far *fsInfo, void far *fInfo, char far *buf, int len);
   int (far *lseek)(void far *fsInfo, void far *fInfo, long offset, int location);
   int (far *ioctl)(void far *fsInfo, void far *fInfo, int op, void far *arg);
   } filesystem_t;

/* File system service routines */
extern int far newFileSystem(filesystem_t far *newFs);
extern int far deleteFileSystem(int fsIndex);

#endif /* _FILESYS_H */

/* End */
