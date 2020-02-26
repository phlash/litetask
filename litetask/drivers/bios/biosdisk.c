/*------------------------------------------------------------------------
   BIOSDISK.C - LiteTask BIOS Disk (INT 13h), installable device driver

   $Author:   Phlash  $
   $Date:   25 Sep 1994 16:47:42  $
   $Revision:   1.4  $

------------------------------------------------------------------------*/

#include "litetask.h"
#include "bios.h"
#include "ioctl.h"

/* Local constants */
#define BD_INT             0x13

/* revision string */
static char pvcsId[] = "$Revision:   1.4  $\r\n";

/* BIOS call lock */
static semaphore_t bcallSem;

/* global installed flag */
static int installed = 0;

/* global drive Id table */
static int devTab[MAX_BIOSDISKS];

/* internal function prototypes */
static int far open(void far *drvInf, long timeout);
static int far close(void far *drvInf);
static int far read(void far *drvInf, void far *buff, int size, long offset);
static int far write(void far *drvInf, void far *buff, int size, long offset);
static int far ioctl(void far *drvInf, int type, void far *buff);
static int near handleBIOSError(int drive, char far *mode, long where, unsigned char code);

/* The user-callable install routine */
int far installBIOSDisk(void)
{
union REGS regs;
int i;
short flag;

/* check for already installed */
   flag = lockTask();
   if(installed)
   {
      unlockTask(flag);
      return 0;
   }
   installed = 1;

/* initialise the semaphore */
   initSemaphore(&bcallSem);
   
/* clear device handle table */
   for(i=0; i<MAX_BIOSDISKS; i++)
      devTab[i] = -1;
   unlockTask(flag);

/* now reset (to test) the BIOS disk system (0x80 means reset HD & FD) */
   if(getSemaphore(&bcallSem, NOTIMEOUT))
   {
      printk("BD: install() Error aquiring BIOS semaphore\r\n");
      return EINTERNAL;
   }
   regs.x.ax = 0;
   regs.x.dx = 0x80;
   int86(BD_INT, &regs);
   putSemaphore(&bcallSem);

/* Say hi! */
   printk("BD: Bios Disk driver (c) (AshbySoft *) 1993-1994: ");
   printk(pvcsId);
   return 0;
}

/* The user-callable remove routine */
int far removeBIOSDisk(void)
{
short flag;
int i;

/* check installed */
   flag = lockTask();
   if(!installed)
   {
      unlockTask(flag);
      return 0;
   }

/* now remove all devices */
   for(i=0; i<MAX_BIOSDISKS; i++)
   {
      if(devTab[i] >= 0)
         deleteBIOSDisk(devTab[i]);
   }

/* clear installed flag */
   installed = 0;
   unlockTask(flag);
   printk("BD: Driver removed\r\n");
   return 0;
}


/* The user-callable device creation routine */
int far createBIOSDisk(int drive)
{
union REGS regs;
driveData_t far *drvInf;
driverInfo_t driver;
int devIdx;
char buf[80];

/* Sanity check */
   switch(drive)
   {
   case 0:
   case 1:
      devIdx = drive;
      break;
   case 0x80:
   case 0x81:
      devIdx = drive - 0x80 + 2;
      break;
   default:
      return EARGS;
   }

/* aquire BIOS call semaphore */
   if(getSemaphore(&bcallSem, NOTIMEOUT))
   {
      sprintf(buf, "BD%i: create() Error aquiring BIOS semaphore\r\n", drive);
      printk(buf);
      return EINTERNAL;
   }

/* now to see if the specified drive exists, get the disk type */
   regs.x.ax = 0x1500;
   regs.x.dx = drive;
   if(int86(BD_INT, &regs))
   {
      putSemaphore(&bcallSem);
      sprintf(buf, "BD%i: create() BIOS error code %i\r\n", drive, regs.h.ah);
      printk(buf);
      return EINTERNAL;
   }
   switch(regs.h.ah)
   {
   case 0:
      putSemaphore(&bcallSem);
      sprintf(buf, "BD%i: Drive not present.\r\n", drive);
      printk(buf);
      return EBADDEV;
   case 1:
   case 2:
      sprintf(buf, "BD%i: Floppy disk. ", drive);
      printk(buf);
      break;
   case 3:
      sprintf(buf, "BD%i: Fixed disk. Size: %l ",
         drive, ((DWORD)regs.x.cx << 16) + (DWORD)regs.x.dx);
      printk(buf);
      break;
   default:
      putSemaphore(&bcallSem);
      sprintf(buf, "BD%i: create() Unknown disk type %i!\r\n",
         drive, regs.h.ah);
      printk(buf);
      return EINTERNAL;
   }

/* allocate drive data block */
   if((drvInf = malloc(sizeof(driveData_t))) == NULL)
   {
      putSemaphore(&bcallSem);
      sprintf(buf, "\r\nBD%i: create() Out of memory\r\n", drive);
      printk(buf);
      return EINTERNAL;
   }

/* store drive number and disk geometry */
   regs.x.ax = 0x0800;
   regs.x.dx = drive;
   if(int86(BD_INT, &regs))
   {
      putSemaphore(&bcallSem);
      free(drvInf);
      sprintf(buf, "\r\nBD%i: create() BIOS error code %i\r\n",
         drive, regs.h.ah);
      printk(buf);
      return EINTERNAL;
   }
   drvInf->drive = drive;
   drvInf->cyls = (WORD)regs.h.ch + ((WORD)(regs.h.cl & 0xC0) << 2) + 1;
   drvInf->heads = (WORD)regs.h.dh + 1;
   drvInf->sectors = (WORD)(regs.h.cl & 0x3F);

/* Dump geometry to console */
   sprintf(buf, "Cyls: %i ", drvInf->cyls);
   printk(buf);
   sprintf(buf, "Heads: %i ", drvInf->heads);
   printk(buf);
   sprintf(buf, "Scts: %i. ", drvInf->sectors);
   printk(buf);

/* Release BIOS call semaphore */
   putSemaphore(&bcallSem);

/* Build driver information structure, and add device to IO system */
   driver.type   = BLK_DEV;
   driver.drvInfo= drvInf;
   driver.open   = open;
   driver.close  = close;
   driver.read   = read;
   driver.write  = write;
   driver.ioctl  = ioctl;
   if((devTab[devIdx] = newIOSysDevice(-1, &driver)) < 0)
   {
      free(drvInf);
      sprintf(buf, "\r\nBD%i: create() Cannot create IO system device\r\n",
         drive);
      printk(buf);
      return EINTERNAL;
   }
   sprintf(buf, " Device=%i\r\n", devTab[devIdx]);
   printk(buf);
   return devTab[devIdx];
}


int far deleteBIOSDisk(int devId)
{
int i;
short flag;
char msg[40];

/* Sanity check */
   flag = lockTask();
   for(i=0; i<MAX_BIOSDISKS; i++)
   {
      if(devId == devTab[i])
         break;
   }
   if(i >= MAX_BIOSDISKS)
   {
      unlockTask(flag);
      return EARGS;
   }


/* Clear table entry */
   devTab[i] = -1;

/* Delete data block */
   free(getDeviceData(devId));

/* Remove from IO system */
   deleteIOSysDevice(devId);
   unlockTask(flag);
   sprintf(msg, "BD%i: Device removed\r\n", (i > 1) ? (i+ 0x80 - 2) : i );
   printk(msg);
   return 0;
}


int far open(driveData_t far *drvInf, long timeout)
{
union REGS regs;
int rv;

/* reset the drive in question */
   switch(getSemaphore(&bcallSem, timeout))
   {
   case 0:
      break;
   case ESEMTIMEOUT:
      return ETIMEOUT;
   default:
      return EINTERNAL;
   }
   regs.x.ax = 0;
   regs.x.dx = drvInf->drive;
   rv = int86(BD_INT, &regs);
   putSemaphore(&bcallSem);
   return rv;
}

int far close(driveData_t far *drvInf)
{
/* Flush data cache? */
   return 0;
}

int far read(driveData_t far *drvInf, void far *buff, int size, long offset)
{
union REGS regs;
char buf[80];
WORD cyl;
int rv, cnt = 0;

/* Read the disk 1 sector at a time (yuk!) */
   while(size--)
   {
      if(getSemaphore(&bcallSem, NOTIMEOUT))
      {
         sprintf(buf, "BD%i: read() Error aquiring BIOS semaphore\r\n",
            drvInf->drive);
         printk(buf);
         if(cnt)
            return cnt;
         else
            return EINTERNAL;
      }
      regs.x.ax = 0x0201;
      regs.h.dl = (BYTE)drvInf->drive;
      cyl = (WORD)(offset / ((long)drvInf->heads * (long)drvInf->sectors));
      regs.h.ch = (BYTE)(cyl & 0xFF);
      regs.h.cl = (BYTE)(cyl >> 2) & 0xC0;
      regs.h.dh = (BYTE)((offset / (long)drvInf->sectors) % (long)drvInf->heads);
      regs.h.cl |= (BYTE)((offset % (long)drvInf->sectors) + 1) & 0x3F;
      regs.x.es = FP_SEG(buff);
      regs.x.bx = FP_OFF(buff);
      rv=int86(BD_INT, &regs);
      putSemaphore(&bcallSem);
      if(rv && (rv=handleBIOSError(drvInf->drive, "read", offset, regs.h.ah)))
         return rv;
      FP_SEG(buff) += 0x20;
      offset++;
      cnt++;
   }
   return cnt;
}

int far write(driveData_t far *drvInf, void far *buff, int size, long offset)
{
union REGS regs;
char buf[80];
WORD cyl;
int rv, cnt = 0;

/* Write the disk 1 sector at a time (yuk!) */
   while(size--)
   {
      if(getSemaphore(&bcallSem, NOTIMEOUT))
      {
         sprintf(buf, "BD%i: write() Error aquiring BIOS semaphore\r\n",
            drvInf->drive);
         printk(buf);
         if(cnt)
            return cnt;
         else
            return EINTERNAL;
      }
      regs.x.ax = 0x0301;
      regs.h.dl = (BYTE)drvInf->drive;
      cyl = (WORD)(offset / ((long)drvInf->heads * (long)drvInf->sectors));
      regs.h.ch = (BYTE)(cyl & 0xFF);
      regs.h.cl = (BYTE)(cyl >> 2) & 0xC0;
      regs.h.dh = (BYTE)((offset / (long)drvInf->sectors) % (long)drvInf->heads);
      regs.h.cl |= (BYTE)((offset % (long)drvInf->sectors) + 1) & 0x3F;
      regs.x.es = FP_SEG(buff);
      regs.x.bx = FP_OFF(buff);
      rv=int86(BD_INT, &regs);
      putSemaphore(&bcallSem);
      if(rv && (rv=handleBIOSError(drvInf->drive, "write", offset, regs.h.ah)))
         return rv;
      FP_SEG(buff) += 0x20;
      offset++;
      cnt++;
   }
   return cnt;
}

int far ioctl(driveData_t far *drvInf, int type, void far *buff)
{
driveData_t far *geom;
short flag;
char buf[80];

/* take appropriate action */
   switch(type)
   {
   case BDIOCSETGEOM:
      geom = (driveData_t far *)buff;
      flag = lockTask();
      drvInf->cyls = geom->cyls;
      drvInf->heads = geom->heads;
      drvInf->sectors = geom->sectors;
      unlockTask(flag);
      sprintf(buf, "BD%i: Geometry changed to: ", drvInf->drive);
      printk(buf);
      sprintf(buf, "Cyls: %i ", drvInf->cyls);
      printk(buf);
      sprintf(buf, "Heads: %i ", drvInf->heads);
      printk(buf);
      sprintf(buf, "Sectors: %i\r\n", drvInf->sectors);
      printk(buf);
      break;
   case BDIOCGETGEOM:
      geom = (driveData_t far *)buff;
      geom->cyls = drvInf->cyls;
      geom->heads = drvInf->heads;
      geom->sectors = drvInf->sectors;
      break;
   case BDIOCFORMAT:
      sprintf(buf, "BD%i: ioctl() BDIOCFORMAT not supported yet.\r\n",
         drvInf->drive);
      printk(buf);
      return EBADIOCTL;
   default:
      sprintf(buf, "BD%i: ioctl() unrecognised ioctl %i.\r\n",
         drvInf->drive, type);
      printk(buf);
      return EBADIOCTL;
   }
   return 0;
}

int near handleBIOSError(int drive, char far *mode, long where, unsigned char code)
{
int rv;
char far *err, buf[80];

   switch(code)
   {
   case 0x0:                         // OK
   case 0x6:                         // Disk changed
      return 0;
   case 0x1:
      err = "Invalid BIOS command";
      rv = EINTERNAL;
      break;
   case 0x2: 
      err = "Address mark not found";
      rv = EINTERNAL;
      break;
   case 0x3:
      err = "Write protected disk";
      rv = EINTERNAL;
      break;
   case 0x4:
      err = "Sector not found";
      rv = EINTERNAL;
      break;
   case 0x5:
      err = "Reset failed";
      rv = EINTERNAL;
      break;
   case 0x7:
      err = "Bad parameter table";
      rv = EINTERNAL;
      break;
   case 0x8:
      err = "DMA Overrun";
      rv = EINTERNAL;
      break;
   case 0x9:
      err = "DMA Crossed 64k boundary";
      rv = EINTERNAL;
      break;
   case 0xA:
      err = "Bad sector flag";
      rv = EINTERNAL;
      break;
   case 0xB:
      err = "Bad track flag";
      rv = EINTERNAL;
      break;
   case 0xC:
      err = "Media type not found";
      rv = EINTERNAL;
      break;
   case 0xD:
      err = "Invalid number of sectors during format";
      rv = EINTERNAL;
      break;
   case 0xE:
      err = "Control data address mark found";
      rv = EINTERNAL;
      break;
   case 0xF:
      err = "DMA arbitration level out of range";
      rv = EINTERNAL;
      break;
   case 0x10:
      err = "Uncorrectable CRC/ECC error";
      rv = EINTERNAL;
      break;
   case 0x11:
   /* 
    * Warn about ECC corrections, the disk is getting dodgy!
    */
      sprintf(buf, "BD%i: %s(%l) ECC corrected disk error\r\n",
         drive, mode, where);
      printk(buf);
      return 0;
   case 0x20:
      err = "Disk controller failed";
      rv = EINTERNAL;
      break;
   case 0x40:
      err = "Seek failed";
      rv = EINTERNAL;
      break;
   case 0x80:
      err = "Disk timed out";
      rv = ETIMEOUT;
      break;
   case 0xAA:
      err = "Drive not ready";
      rv = ETIMEOUT;
      break;
   case 0xBB:
      err = "Undefined error";
      rv = EINTERNAL;
      break;
   case 0xCC:
      err = "Write fault";
      rv = EINTERNAL;
      break;
   case 0xE0:
      err = "Status register error";
      rv = EINTERNAL;
      break;
   case 0xFF:
      err = "Sense operation failed";
      rv = EINTERNAL;
      break;
   default:
      sprintf(buf, "BD%i: %s(%l) Unknown BIOS error code %x\r\n",
         drive, mode, where, code);
      printk(buf);
      return EINTERNAL;
   }
   sprintf(buf, "BD%i: %s(%l) %s\r\n", drive, mode, where, err);
   printk(buf);
   return rv;
}

/* End */
