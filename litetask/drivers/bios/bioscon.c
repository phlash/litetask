/*------------------------------------------------------------------------
   BIOSCON.C - LiteTask BIOS (INT 10h/16h) console device driver

   $Author:   Phlash  $
   $Date:   04 Jun 1994 20:28:26  $
   $Revision:   1.4  $

------------------------------------------------------------------------*/

#include "litetask.h"
#include "bios.h"
#include "ioctl.h"

/* revision string */
static char pvcsId[] = "$Revision:   1.4  $\r\n";

/* installed flag */
static int installed = 0;

/* Device id table & current focus */
static int devTab[MAX_CONSOLES];
static int focusDev = 0;

/* read task handle */
static taskHandle readHandle = NULL;

/* internal function prototypes */
static int far read(void far *drvInf, void far *buff, int size, long offset);
static int far write(void far *drvInf, void far *buff, int size, long offset);
static int far ioctl(void far *drvInf, int type, void far *buff);

static void far _readTask(void);

static void near switchPage(int page);
static void near putch(consoleData_t far *conInf, int ch);

/* The user-callable install / create / delete & remove routines */
int far installConsole(void)
{
int i;
short flag;

/* check for already installed */
   flag = lockTask();
   if(!installed)
   {
   /* Say Hi */
      installed = 1;
      for(i=0; i<MAX_CONSOLES; i++)
         devTab[i] = -1;
      readHandle = newTask(MINSTACK, _readTask, 0, NULL);
      if(!readHandle)
      {
         unlockTask(flag);
         printk("BC: Cannot create read task\r\n");
         return EINTERNAL;
      }
      printk("BC: Bios Console driver (c) (AshbySoft *) 1993-1994 ");
      printk(pvcsId);
   }
   unlockTask(flag);
   return 0;
}

int far createConsole(int BIOSPage)
{
consoleData_t far *newCon;
driverInfo_t driverInfo;
union REGS regs;
short flag;
char msg[40];

/* Sanity check */
   if(BIOSPage < 0 || BIOSPage >= MAX_CONSOLES)
      return EARGS;

/* check previous create */
   flag = lockTask();
   if(devTab[BIOSPage] >= 0)
   {
      unlockTask(flag);
      printk("BC: Attempt to create previously created device\r\n");
      return EARGS;
   }

/* allocate the device data structure */
   newCon = (consoleData_t far *) malloc( sizeof(consoleData_t) );
   if(!newCon)
   {
      unlockTask(flag);
      printk("BC: Out of memory creating device\r\n");
      return EINTERNAL;
   }

/* build the driverInfo_t structure */
   driverInfo.type  = CHR_DEV;
   driverInfo.drvInfo=newCon;
   driverInfo.open  = NULL;
   driverInfo.close = NULL;
   driverInfo.read  = read;
   driverInfo.write = write;
   driverInfo.ioctl = ioctl;

/* initialise device settings */
   newCon->page = BIOSPage;
   newCon->mode = BC_ECHO | BC_BLK | BC_COOKED;
   newCon->head = newCon->tail = 0;
   newCon->pendTask = NULL;
   newCon->selList = NULL;

/* read current cursor position (I feel nice today) */
   regs.h.ah = 0x03;
   regs.h.bh = BIOSPage;
   int86(0x10, &regs);
   newCon->cx = regs.h.dl;
   newCon->cy = regs.h.dh;

/* install the driver */
   if((devTab[BIOSPage]=newIOSysDevice(-1, &driverInfo)) < 0)
   {
      unlockTask(flag);
      printk("BC: Cannot install console device driver\r\n");
      return EINTERNAL;
   }

/* OK! */
   unlockTask(flag);
   sprintf(msg, "BC%i: Installed as device %i\r\n", BIOSPage, devTab[BIOSPage]);
   printk(msg);

/* All OK */
   return devTab[BIOSPage];
}

int far deleteConsole(int devId)
{
int i;
short flag;
char msg[40];

/* Have we installed as this device? */
   flag = lockTask();
   for(i=0; i<MAX_CONSOLES; i++)
   {
      if(devId == devTab[i])
         break;
   }
   if(i >= MAX_CONSOLES)
   {
      unlockTask(flag);
      printk("BC: attempt to delete invalid device\r\n");
      return EBADDEV;
   }

/* Clear table entry */
   devTab[i] = -1;

/* free the data block */
   free( getDeviceData(devId) );

/* Delete the device from the IO system */
   deleteIOSysDevice(devId);

/* Done */
   unlockTask(flag);
   sprintf(msg, "BC%i: Device removed\r\n", i);
   printk(msg);
   return 0;
}

int far removeConsole(void)
{
int i;
short flag;

/* check installed */
   flag = lockTask();
   if(!installed)
   {
      unlockTask(flag);
      return 0;
   }

/* remove the devices */
   for(i=0; i<MAX_CONSOLES; i++)
   {
      if(devTab[i] >= 0)
         deleteConsole(devTab[i]);
   }

/* clear the installed flag (read task checks this) */
   installed = 0;
   unlockTask(flag);

/* wait for termination */
   printk("BC: Please press a key..");
   waitTask(readHandle, NOTIMEOUT);

/* switch back to page 0 */
   switchPage(focusDev = 0);

/* Done */
   printk("Driver removed\r\n");
   return 0;
}

/* *** internal driver functions *** */

static int far read(void far *drvInf, void far *buff, int size, long offset)
{
consoleData_t far *conInf = (consoleData_t far *)drvInf;
char far *buf = (char far *)buff;
int cnt, ch;
short flag;

/* read loop */
   for(cnt=0; cnt<size; cnt++)
   {
   /* see if we have any data in the buffer */
      flag = lockTask();
      if(conInf->tail == conInf->head)
      {
      /* Empty buffer, do we block or return? */
         if(conInf->mode & BC_BLK)
         {
            conInf->pendTask = getTaskHandle();
            suspendTask();
            unlockTask(flag);
         }
         else
         {
            unlockTask(flag);
            return cnt;
         }
      }

   /* read keystroke from buffer */
      ch = conInf->keyBuf[conInf->tail++];
      if(conInf->tail >= KEY_BUF_SIZE)
         conInf->tail = 0;
      unlockTask(flag);

   /* process keystroke */
      switch(ch)
      {
      case '\b':               // Backspace deletes (COOKED mode)
         if(conInf->mode & BC_COOKED)
         {
            if(cnt)
               cnt-=2;
            else
               ch = 7;
         }
         break;

      case '\n':               // CR or LF terminates read (COOKED mode).
      case '\r':
         if(conInf->mode & BC_COOKED)
         {
            ch = '\n';
            size = 0;          // Yuk! but I must echo & I can't change cnt :)
         }
      
      /* *** Fall through in RAW mode *** */

      default:                 // Anything else just store in buffer
         buf[cnt] = (char)ch;
         break;
      }
      if(conInf->mode & BC_ECHO)
         write(drvInf, &ch, 1, 0L);
   }
   return cnt;                 // Buffer full, terminate read
}

static int far write(void far *drvInf, void far *buff, int size, long offset)
{
consoleData_t far *conInf = (consoleData_t far *)drvInf;
char far *buf = (char far *)buff;
int cnt, i;

/* write loop */
   for(cnt=0; cnt<size; cnt++)
   {
      switch(buf[cnt])
      {
      case '\n':
         if(conInf->mode & BC_COOKED) // Add CR if in cooked mode
            putch(conInf, '\r');
         putch(conInf, buf[cnt]);
         break;

      case '\f':                      // Form feed has to occur here (oops!)
         putch(conInf, '\r');
         for(i=0; i<SCREEN_HEIGHT; i++)
            putch(conInf, '\n');
         break;
      
      default:                        // All normal characters
         putch(conInf, buf[cnt]);
         break;
      }
   }
   return size;
}

static int far ioctl(void far *drvInf, int type, void far *buff)
{
consoleData_t far *conInf = (consoleData_t far *)drvInf;
selectInfo_t far *selInf;
long tmp;
short flag;
char buf[40];

/* get/set appropriate bit(s) in mode flag */
   switch(type)
   {
   case BCIOCGETMODE:
      *((int far *)buff) = conInf->mode;
      break;

   case BCIOCSETMODE:
      conInf->mode = *((int far *)buff);
      break;

   case BCIOCECHO:
      conInf->mode |= BC_ECHO;
      break;

   case BCIOCNOECHO:
      conInf->mode &= ~BC_ECHO;
      break;

   case BCIOCCOOKED:
      conInf->mode |= BC_COOKED;
      break;

   case BCIOCRAW:
      conInf->mode &= ~BC_COOKED;
      break;

   case STDIOCBLK:
      conInf->mode |= BC_BLK;
      break;

   case STDIOCNBLK:
      conInf->mode &= ~BC_BLK;
      break;

   case STDIOCNREAD:
      flag = lockTask();
      tmp = (long)(conInf->head - conInf->tail);
      unlockTask(flag);
      if(tmp < 0L)
         tmp += (long)KEY_BUF_SIZE;
      *((long far *)(buff)) = tmp;
      break;

   case STDIOCSELECT:
      flag = lockTask();
      selInf = (selectInfo_t far *)buff;
      selInf->next = conInf->selList;
      selInf->prev = NULL;
      conInf->selList = selInf;
      unlockTask(flag);
      break;

   case STDIOCNSELECT:
      flag = lockTask();
      selInf = (selectInfo_t far *)buff;
      if(selInf->next)
         selInf->next->prev = selInf->prev;
      if(selInf->prev)
         selInf->prev->next = selInf->next;
      else
         conInf->selList = selInf->next;
      unlockTask(flag);
      break;

   default:
      sprintf(buf, "BC: Invalid ioctl %i\r\n", type);
      printk(buf);
      return EBADIOCTL;
   }
   return 0;
}

/* Spawned keyboard reader task */
void far _readTask(void)
{
union REGS regs;
int key;
consoleData_t far *conInf;
selectInfo_t far *selInf;
short flag;

/* main read loop */
   for(;;)
   {
   /* check the installed flag (termination request) */
      flag = lockTask();
      if(!installed)
      {
         unlockTask(flag);
         break;
      }
      unlockTask(flag);

   /* don't block in BIOS unless INT 15h handler installed */
      if(!checkInt15())
      {
      /* poll the BIOS instead, yukky but safe */
         do
         {
            yieldTask();
            regs.x.ax = 0x100;
            int86(0x16, &regs);
         }
         while(regs.x.flags & 0x40);
      }

   /* read waiting keypress */
      regs.x.ax = 0;
      int86(0x16, &regs);
      if(!regs.h.al)         // Extended key press, set high bit in key
      {                      // and check scan code in ah
         key = 0x8000;
   
      /* Ctrl+F<n> selects new console (no keycodes generated) */
         if((regs.h.ah >= 0x5E) && (regs.h.ah < 0x66))
         {
            flag = lockTask();
            focusDev = (int)regs.h.ah - 0x5E;
            switchPage(focusDev);
            unlockTask(flag);
            continue;
         }
         else
         {
            key |= (int)regs.h.ah & 0xFF;
         }
      }
      else
         key = (int)regs.h.al & 0xFF;

   /* place key code in keyboard buffer & wake pending task */
      flag = lockTask();
      conInf = (consoleData_t far *)getDeviceData(devTab[focusDev]);
      if(conInf)
      {
         conInf->keyBuf[conInf->head++] = key;
         if(conInf->head >= KEY_BUF_SIZE)
            conInf->head = 0;
         if(conInf->pendTask)
         {
            resumeTask(conInf->pendTask, 0);
            conInf->pendTask = NULL;
         }
         if(conInf->selList)
         {
            for(selInf = conInf->selList; selInf; selInf = selInf->next)
               resumeTask(selInf->task, devTab[focusDev]);
         }
      }
      unlockTask(flag);
   }
}

void near switchPage(int page)
{
union REGS regs;

/* INT 10h sub function 05h, change displayed video page */
   regs.h.ah = 0x05;
   regs.h.al = page;
   int86(0x10, &regs);
}

void near putch(consoleData_t far *conInf, int ch)
{
union REGS regs;
short flag;

   switch(ch)
   {
   case 0:            // Special used for cursor re-positioning only
      break;

   case '\b':
      if(conInf->cx)
      {
         conInf->cx--;
         putch(conInf, 0);
         putch(conInf, ' ');
         conInf->cx--;
         putch(conInf, 0);
      }
      else
         putch(conInf, 7);
      break;

   case '\r':
      conInf->cx = 0;
      break;

   case '\n':
      conInf->cy++;
      break;

   case '\t':
      while(++(conInf->cx) % DEFAULT_TABSIZE);
      break;

   case 7:
      outp(0x61, inp(0x61) ^ 0x03);
      delayTask(2L);
      outp(0x61, inp(0x61) ^ 0x03);
      break;

   default:
   /* INT 10h sub function 09h, write char and attr at cursor */
      regs.h.ah = 0x09;
      regs.h.al = ch;
      regs.h.bh = conInf->page;
      regs.h.bl = 0x07;
      regs.x.cx = 1;
      int86(0x10, &regs);
      conInf->cx++;
      break;
   }

/* check cursor position (possibly scroll screen) */
   if(conInf->cx >= SCREEN_WIDTH)
   {
      conInf->cx = 0;
      conInf->cy++;
   }
   if(conInf->cy >= SCREEN_HEIGHT)
   {
   /* Scroll required, use INT 10h sub function 06h, scroll up */
      conInf->cy = SCREEN_HEIGHT-1;
      flag = lockTask();
      switchPage(conInf->page);
      regs.h.ah = 06;
      regs.h.al = 1;
      regs.h.bh = 7;
      regs.x.cx = 0;
      regs.h.dh = SCREEN_HEIGHT-1;
      regs.h.dl = SCREEN_WIDTH-1;
      int86(0x10, &regs);
      switchPage(focusDev);
      unlockTask(flag);
   }

/* update cursor position */
   regs.h.ah = 0x02;
   regs.h.bh = conInf->page;
   regs.h.dh = conInf->cy;
   regs.h.dl = conInf->cx;
   int86(0x10, &regs);
}

/* End */
