/*-----------------------------------------------------------------------
   LITETASK.H - Main Header file for "LiteTask"
   
   A Multi-Tasking kernel for IBM-PC's and compatibles.
   
   Copyright (c) 1992-1993 by (AshbySoft *)

   $Author:   Phlash  $
   $Date:   16 Jul 1995  9:24:24  $
   $Revision:   1.24  $

-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
   Kernel Version
-----------------------------------------------------------------------*/
#define LITETASK_VERSION 0x0005

/*-----------------------------------------------------------------------
   ANSI C Library
-----------------------------------------------------------------------*/
#ifndef _CLIB_H
#include <clib.h>
#endif

/*-----------------------------------------------------------------------
   Task Manager API
-----------------------------------------------------------------------*/

/* Task Information structure */
typedef struct taskInfo_tag {
                           char magic[4];
                           struct taskInfo_tag *next;
                           struct taskInfo_tag *prev;
                           void *stack;
                           void *context;
                           unsigned int taskState;
                           long extra;
                           } taskInfo_t;

/* Task Handle type */
typedef taskInfo_t *taskHandle;

/* Task creation & deletion */
extern taskHandle newTask(unsigned int stacksize,
                          void *function, int arg_size, ...);
extern int waitTask(taskHandle taskID, long waitTicks);
extern int deleteTask(taskHandle taskID);
extern void taskExit(int);

/* Task self-identification */
extern taskHandle getTaskHandle(void);

/* Task interaction controls */
extern short lockTask(void);
extern void unlockTask(short flag);
extern void yieldTask(void);
extern int suspendTask(void);
extern int resumeTask(taskHandle taskID, int resumeStatus);
extern int delayTask(long delayTicks);

/* Idle task hook */
#define MAX_IDLEHOOKS   16
extern int setIdleHook(void (*hookFunction)(void));
extern int clearIdleHook(void (*hookFunction)(void));

/* Task statistics */
extern int getTaskCount(void);
extern int getLockOuts(void);
extern int getTaskSwitches(void);
extern int getIdleSwitches(void);
extern int getScheduleTime(void);
extern int traceTaskStack(taskHandle taskID);


/*-----------------------------------------------------------------------
   Timer Manager API
-----------------------------------------------------------------------*/

/* Timer Information structure */
typedef struct timerInfo_tag {
               char magic[4];
               struct timerInfo_tag *next;
               short status;
               long count;
               long arg;
               void (*function)(long arg);
               } timerInfo_t;

/* Timer Handle type */
typedef timerInfo_t *timerHandle;

/* timer status values */
#define WDT_DONE     0
#define WDT_ACTIVE   1

/* Timer install/remove */
extern int installTimer(unsigned short timerMax);
extern void removeTimer(void);

/* Timer pre-emption control */
extern void setPreEmptive(int ticks);

/* Profiler support */
extern int enableProf(void *addr, WORD *pBuf, WORD len);
extern void disableProf(void);

/* Timer readouts */
extern long getTicks(void);      // LiteTask system ticks (programmable)
extern unsigned short getTimer(void); // PC Timer chip clocks (1.193 MHz)

/* Timer creation & deletion */
extern timerHandle newTimer(void);
extern int deleteTimer(timerHandle timerID);

/*
   Timer start & stop controls
   NOTE: These can be used safely in interrupt handlers
*/
extern int startTimer(timerHandle timer, long delay,
               void (*function)(long), long argument);
extern int stopTimer(timerHandle timerID);


/*-----------------------------------------------------------------------
   Semaphore Library API
-----------------------------------------------------------------------*/

/* A semaphore queue entry */
typedef struct semQueue_tag {
            struct semQueue_tag *qnext, *qprev;
            struct semQueue_tag *tnext, *tprev;
            struct semaphore_tag *psem;
            taskHandle task;
            long remaining;
            } semQueue_t;

/* A semaphore */
typedef struct semaphore_tag {
            char magic[4];
            taskHandle owner;
            semQueue_t *qhead, *qtail;
            } semaphore_t;

/* Initialise, aquire & release operations */
extern void initSemaphore(semaphore_t *newSem);
extern int getSemaphore(semaphore_t *sem, long timeout);
extern int putSemaphore(semaphore_t *sem);

/*-----------------------------------------------------------------------
   IO System (device drivers) API
-----------------------------------------------------------------------*/

/* Device Driver Information structure */
typedef struct driverInfo_tag {
               int type;
               void *drvInfo;
               int (*open) (void *drvInfo, long timeout);
               int (*close)(void *drvInfo);
               int (*read) (void *drvInfo, void *buff, int size, long offset);
               int (*write)(void *drvInfo, void *buff, int size, long offset);
               int (*ioctl)(void *drvInfo, int type, void *buff);
               } driverInfo_t;

/* Values for type field */
#define FREE_DEV  0
#define CHR_DEV   1
#define BLK_DEV   2

/* Device creation & deletion (Driver API) */
extern int newIOSysDevice(int device, driverInfo_t *drvInfo);
extern int deleteIOSysDevice(int device);
extern void *getDeviceData(int device);

/* Device IO (Application API) */
extern int openDevice(int device, long timeout);
extern int closeDevice(int device);
extern int readDevice(int device, void *buff, int size, long offset);
extern int writeDevice(int device, void *buff, int size, long offset);
extern int ioctlDevice(int device, int type, void *buff);
extern int getDeviceType(int device);

/*-----------------------------------------------------------------------
   Video Driver Library API
-----------------------------------------------------------------------*/

/* The text driver interface structure (declare one in your driver :) */
typedef struct {
         char *description;
         int width, height;
         int  (*Init)(void);
         void (*Remove)(void);
         void (*OutCh)(int c);
         void (*OutStr)(char *str);
         void (*OutChXY)(int x, int y, int c);
         void (*OutStrXY)(int x, int y, char *str);
         void (*Clear)(void);
         void (*Scroll)(int lines);
         int  (*Ctrl)(int ctrl, void *arg);
         } textDriver_t;

/* The default text mode driver (and it's Ctrl's) */
extern textDriver_t textDrv;
#define TEXT_CTRL_RESET       0
#define TEXT_CTRL_CGASCREEN   1
#define TEXT_CTRL_MDASCREEN   2

/* Colormap entry structure */
typedef struct {
         unsigned long pixel;
         unsigned long rgb;
         } cmap_t;

/* The graphics driver interface structure (declare one in your driver :) */
typedef struct {
         char *description;
         unsigned short width, height, planes;
         void (*InitDriver)(void);
         void (*RemoveDriver)(void);
         void (*DrawPoint)(unsigned short x, unsigned short y,
                           unsigned long pix, unsigned long pmask);
         void (*DrawLine)(unsigned short x1, unsigned short y1,
                           unsigned short x2, unsigned short y2,
                           unsigned long pix, unsigned long pmask);
         void (*DrawRectangle)(unsigned short x1, unsigned short y1,
                           unsigned short x2, unsigned short y2,
                           unsigned long pix, unsigned long pmask);
         void (*FillRectangle)(unsigned short x1, unsigned short y1,
                           unsigned short x2, unsigned short y2,
                           unsigned long pix, unsigned long pmask);
         void (*DrawString)(unsigned short x, unsigned short y, char *str,
                           int len, unsigned long pix, unsigned long pmask);
         void (*DrawPixmap)(unsigned short x, unsigned short y,
                           unsigned short w, unsigned short h,
                           unsigned long *pixmap, unsigned long pmask);
         void (*DrawBitmap)(unsigned short x, unsigned short y,
                           unsigned short w, unsigned short h,
                           unsigned char *bitmap,
                           unsigned long pix, unsigned long pmask);
         unsigned long (*GetPixel)(unsigned short x, unsigned short y,
                           unsigned long pmask);
         void (*ChangeColormap)(unsigned short ncols, cmap_t *cols);
         } primsDriver_t;

/* Pixmap types */
#define PIXMAP 0             /* Pixmap contains unsigned long pixel values */
#define BITMAP 1             /* Pixmap contains 1 bit/pixel bit string */

/* Graphics drivers */
extern primsDriver_t vm18drv;
extern primsDriver_t vm19drv;

/*-----------------------------------------------------------------------
   File System Library API
-----------------------------------------------------------------------*/

/* Open modes */
#define O_RDONLY  0
#define O_WRONLY  1
#define O_RDWR    2

/* Open flags */
#define O_CREAT   4
#define O_TRUNC   8

/* Seek modes */
#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2

extern int open(char *path, int mode, ...);
extern int close(int fd);
extern int read(int fd, char *buf, int len);
extern int write(int fd, char *buf, int len);
extern int lseek(int fd, long offset, int location);
extern int ioctl(int fd, int op, void *arg);

/* generic file system routines */
extern int initFileSystem(void);
extern int removeFileSystem(void);

/* Individual file systems: */

/* File Allocation Table (FAT) or MS-DOS file system */
extern int initFatFs(void);
extern int removeFatFs(void);
extern int newFatFs(int device, long offset, char *name);
extern int deleteFatFs(int fsId);

/*-----------------------------------------------------------------------
   Kernel Message Output / Buffering API
-----------------------------------------------------------------------*/

extern int printk(char *fmt, ...);
extern void setPrintk(textDriver_t *drv, char *buf, int size);

/*-----------------------------------------------------------------------
   Low-level Kernel Routines
-----------------------------------------------------------------------*/

/* task switcher stuff */
extern void scheduleEntry(void);
extern void quitScheduler(int quitCode);
extern void *newContext(void *stacktop, void *taskAddress);

/* stack paranoia */
extern int traceHWStack(int irq);
extern int traceTaskStack(taskHandle task);

/* Profiler support */
extern int startProfiling(void);
extern int stopProfiling(char *saveFile);

/*-----------------------------------------------------------------------
   Constants
-----------------------------------------------------------------------*/

/* Task Mgr errors */
#define ETASKID      -1
#define ETIMEOUT     -2
#define EARGS        -3
#define ETASKSTATE   -4
#define EKILLED      -5
#define ENOIDLESPACE -6

/* Timer Mgr errors */
#define ETIMERID     -10

/* IO System errors */
#define EDEVICEID    -20
#define EBADDEV      -21
#define EDEVUSED     -22
#define ENODEVSPACE  -23
#define EBADIOCTL    -24

/* Semaphore library errors */
#define EBADSEM      -30
#define ESEMINUSE    -31
#define ESEMTIMEOUT  -32

/* Memory library errors */
#define EBADMCB      -40

/* Something ghastly has happened.... */
#define EINTERNAL    -100

/* other miscellaneous constants */
#define NOTIMEOUT    -1L
#define MINSTACK     1024
#define MAXSTACK     32766
#define MINSTACKLEFT 32

/*-----------------------------------------------------------------------
   Main Task Declarations
-----------------------------------------------------------------------*/

extern int mainStackSize;     
extern void mainTask(char *commandLine);

/* End */
