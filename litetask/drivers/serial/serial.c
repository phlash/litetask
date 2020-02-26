/*------------------------------------------------------------------------
   SERIAL.C - LiteTask Hardware level serial port driver

   $Author:   Phlash  $
   $Date:   27 May 1994 20:48:48  $
   $Revision:   1.3  $

------------------------------------------------------------------------*/

#include "litetask.h"
#include "select.h"
#include "8259.h"
#include "ioctl.h"
#include "serial.h"

/* Pend reasons */
#define PEND_READ    0
#define PEND_WRITE   1

/* revision string */
static char pvcsId[] = "$Revision:   1.3  $\r\n";

/* global installed flag */
static int installed = 0;

/* global device ID table */
static int devTab[MAX_SERIALPORTS];

/* default settings for COM1/COM2 */
static struct {
            int baseAddr;
            int trap;
            int baud;
            } portDefaults[MAX_SERIALPORTS] =
{
   { 0x3F8, 4, 1200 },  /* COM1 */
   { 0x2F8, 3, 9600 }   /* COM2 */
};

/* shorthand macros */
#define COM_TXBUF(comInf) (comInf->baseAddr+0)
#define COM_RXBUF(comInf) (comInf->baseAddr+0)
#define COM_DIVLO(comInf) (comInf->baseAddr+0)
#define COM_DIVHI(comInf) (comInf->baseAddr+1)
#define COM_IER(comInf)   (comInf->baseAddr+1)
#define COM_IIR(comInf)   (comInf->baseAddr+2)
#define COM_LCR(comInf)   (comInf->baseAddr+3)
#define COM_MCR(comInf)   (comInf->baseAddr+4)
#define COM_LSR(comInf)   (comInf->baseAddr+5)
#define COM_MSR(comInf)   (comInf->baseAddr+6)

/* prototypes */
static int far read(void far *drvInf, void far *buff, int size, long offset);
static int far write(void far *drvInf, void far *buff, int size, long offset);
static int far ioctl(void far *drvInf, int type, void far *buff);
static void interrupt far _serialTrap(void);


/* Code! */
int far installSerial(void)
{
int i;
short flag;

/* check installed flag */
   flag = lockTask();
   if(!installed)
   {
      installed = 1;
      for(i=0; i<MAX_SERIALPORTS; i++)
         devTab[i] = -1;
      printk("SER: Serial port driver (c) (AshbySoft *) 1994: ");
      printk(pvcsId);
   }
   unlockTask(flag);
   return 0;
}

int far removeSerial(void)
{
int i;
short flag;

/* remove each device installed */
   flag = lockTask();
   for(i=0; i<MAX_SERIALPORTS; i++)
   {
      if(devTab[i] >= 0)
         deleteSerial(devTab[i]);
   }
   printk("SER: driver removed\r\n");
   unlockTask(flag);
   return 0;
}

int far createSerial(int portId, int mode)
{
serialData_t far *newSer;
driverInfo_t drvInfo;
int int_mask;
unsigned short divisor;
short flag;
char msg[80];

/* sanity checks */
   if(portId >= MAX_SERIALPORTS || portId < 0)
   {
      sprintf(msg, "SER: create(): portId out of range (%i)\r\n", portId);
      printk(msg);
      return EARGS;
   }

/* done already? check */
   flag = lockTask();
   if(devTab[portId] >= 0)
   {
      unlockTask(flag);
      sprintf(msg, "SER: create(): attempt to re-create same device (%i)\r\n",
         portId);
      printk(msg);
      return EARGS;
   }

/* allocate data structure */
   newSer = (serialData_t far *)malloc( sizeof(serialData_t) );
   if(!newSer)
   {
      unlockTask(flag);
      sprintf(msg, "SER%i: create(): out of memory\r\n", portId);
      printk(msg);
      return EINTERNAL;
   }
   newSer->portId   = portId;
   newSer->baseAddr = portDefaults[portId].baseAddr;
   newSer->trap     = portDefaults[portId].trap;
   newSer->baud     = portDefaults[portId].baud;
   newSer->mode     = mode;
   newSer->status   = 0;
   newSer->pendWhy  = 0;
   newSer->pendTask = NULL;
   newSer->selList  = NULL;
   newSer->head = newSer->tail = 0;

/* Check for existance of hardware by writing to LCR */
   outp(COM_LCR(newSer), 0x55);
   if(inp(COM_LCR(newSer)) != 0x55)
   {
      unlockTask(flag);
      free(newSer);
      sprintf(msg, "SER%i: No such hardware\r\n", portId);
      printk(msg);
      return EDEVUSED;
   }
   outp(COM_LCR(newSer), 0);

/* Install in device table */
   drvInfo.type   = CHR_DEV;
   drvInfo.drvInfo= newSer;
   drvInfo.open   = NULL;
   drvInfo.close  = NULL;
   drvInfo.read   = read;
   drvInfo.write  = write;
   drvInfo.ioctl  = ioctl;
   if((devTab[portId] = newIOSysDevice(-1, &drvInfo)) < 0)
   {
      unlockTask(flag);
      sprintf(msg, "SER%i: error installing in device table\r\n", portId);
      printk(msg);
      return ENODEVSPACE;
   }

/* Trap interrupt for this port */
   newSer->oldTrap = setVector(INT_OFFSET + newSer->trap, _serialTrap);

/* Enable interrupts in 8259 */
   int_mask = 1 << newSer->trap;
   outp(INT_MASK_PORT, inp(INT_MASK_PORT) & ~int_mask);

/* Calculate divisor from baud rate */
   divisor = (unsigned short) (115200L/(long)(newSer->baud));

/* Set up port for pre-assigned baud and specified mode */
   outp(COM_LCR(newSer), COM_LCR_DIV);
   outp(COM_DIVLO(newSer), divisor & 0xFF);
   outp(COM_DIVHI(newSer), divisor >> 8);
   outp(COM_LCR(newSer), mode & 0x7F);
   
/* Enable DTR, RTS & OUT2 => 'ready to rock' */
   outp(COM_MCR(newSer), COM_MCR_DTR | COM_MCR_RTS | COM_MCR_OUT2);
   
/* Enable interrupts for all receive state changes */
   outp(COM_IER(newSer), COM_IER_RXDATA | COM_IER_STATUS | COM_IER_MODEM);

/* Say created OK */
   sprintf(msg, "SER%i: created as device %i\r\n", portId, devTab[portId]);
   printk(msg);

/* All OK */
   unlockTask(flag);
   return devTab[portId];
}

int far deleteSerial(int devId)
{
serialData_t far *oldSer;
int i, int_mask;
short flag;
char msg[80];

/* device deletion */
   flag = lockTask();
   for(i=0; i<MAX_SERIALPORTS; i++)
   {
      if(devId == devTab[i])
         break;
   }
   if(i >= MAX_SERIALPORTS)
   {
      unlockTask(flag);
      return EARGS;
   }

/* get data pointer */
   oldSer = (serialData_t far *) getDeviceData(devId);

/* delete from device table */
   deleteIOSysDevice(devId);
   devTab[i] = -1;

/* disable ints from device, then 8259, then unhook trap */
   outp(COM_IER(oldSer), 0);
   outp(COM_MCR(oldSer), 0);
   int_mask = 1 << oldSer->trap;
   outp(INT_MASK_PORT, inp(INT_MASK_PORT) | int_mask);
   setVector(INT_OFFSET + oldSer->trap, oldSer->oldTrap);
   free(oldSer);

/* say done */
   sprintf(msg, "SER%i: device deleted\r\n", i);
   printk(msg);
   unlockTask(flag);
   return 0;
}

/* The internal driver functions */
static int far read(serialData_t far *comInf, void far *buff, int size, long offset)
{
unsigned char far *buf = (char far *)buff;
int cnt;
short flag;
char msg[40];

/* check for any errors */
   if(comInf->status & SER_ERRBITS)
   {
      sprintf(msg, "SER%i: Device status 0x%x\r\n",
         comInf->portId, comInf->status);
      printk(msg);
   }

/* blocking/non-blocking read loop from interrupt buffer */
   for(cnt = 0; cnt < size; cnt++)
   {
   /* check for any data in buffer */
      flag = lockInts();
      if(comInf->head != comInf->tail)
         unlockInts(flag);
      else
      {
      /* block if required */
         if(comInf->mode & SER_MODE_BLK)
         {
            comInf->pendTask = getTaskHandle();
            comInf->pendWhy  = PEND_READ;
            suspendTask(); // wait for incoming data to unblock
            unlockInts(flag);
         }
         else
         {
            unlockInts(flag);
            break;
         }
      }
      buf[cnt] = comInf->buffer[comInf->tail++];
      if(comInf->tail >= SERIAL_BUF_SIZE)
         comInf->tail = 0;
   }
   return cnt;
}

static int far write(serialData_t far *comInf, void far *buff, int size, long offset)
{
unsigned char far *buf = (char far *)buff;
int cnt;
short flag;

/* write to device, pend if not ready */
   for(cnt=0; cnt < size; cnt++)
   {
   /* test & set on THRE status */
      flag = lockInts();
      if(inp(COM_LSR(comInf)) & SER_THRE)
         unlockInts(flag);
      else
      {
      /* block if required */
         if(comInf->mode & SER_MODE_BLK)
         {
            outp(COM_IER(comInf), inp(COM_IER(comInf)) | COM_IER_TXRDY);
            comInf->pendTask = getTaskHandle();
            comInf->pendWhy  = PEND_WRITE;
            suspendTask();
            outp(COM_IER(comInf), inp(COM_IER(comInf)) & ~COM_IER_TXRDY);
            unlockInts(flag);
         }
         else
         {
            unlockInts(flag);
            break;
         }
      }
      outp(COM_TXBUF(comInf), buf[cnt]);
   }
   return cnt;
}

static int far ioctl(serialData_t far *comInf, int type, void far *buff)
{
short flag;
unsigned short divisor;
unsigned char lcr;
selectInfo_t far *selInfo;
char msg[40];

/* get/set appropriate bit(s) in mode flag */
   switch(type)
   {
   case SERIOCGETBAUD:
      *((int far *)buff) = comInf->baud;
      break;

   case SERIOCSETBAUD:
      comInf->baud = *((int far *)buff);
      sprintf(msg, "SER%i: Setting baud rate to %i\r\n",
         comInf->portId, comInf->baud);
      printk(msg);
      divisor = (unsigned short) (115200L/(long)(comInf->baud));

   /* Set up port for specified baud */
      flag = lockTask();
      lcr = inp(COM_LCR(comInf));
      outp(COM_LCR(comInf), lcr | 0x80);
      outp(COM_DIVLO(comInf), divisor & 0xFF);
      outp(COM_DIVHI(comInf), divisor >> 8);
      outp(COM_LCR(comInf), lcr);
      unlockTask(flag);
      break;

   case SERIOCGETSTATUS:
      *((int far *)buff) = comInf->status;
      break;

   case STDIOCBLK:
      comInf->mode |= SER_MODE_BLK;
      break;

   case STDIOCNBLK:
      comInf->mode &= ~SER_MODE_BLK;
      break;

   case STDIOCASYNC:
      comInf->mode |= SER_MODE_ASYNC;
      break;

   case STDIOCNASYNC:
      comInf->mode &= ~SER_MODE_ASYNC;
      break;

   case STDIOCNREAD:
      *((long far *)buff) = (long)(comInf->head - comInf->tail);
      if(*((long far *)buff) < 0L)
         *((long far *)buff) += (long)SERIAL_BUF_SIZE;
      break;

   case STDIOCSELECT:
   /* Insert on queue of selecting tasks */
      selInfo = (selectInfo_t far *)buff;
      flag = lockInts();
      selInfo->next = comInf->selList;
      selInfo->prev = NULL;
      comInf->selList->prev = selInfo;
      comInf->selList = selInfo;
      unlockInts(flag);
      break;

   case STDIOCNSELECT:
   /* Remove entry from queue of selecting tasks */
      selInfo = (selectInfo_t far *)buff;
      flag = lockInts();
      if(selInfo->prev)
         selInfo->prev->next = selInfo->next;
      else
         comInf->selList = selInfo->next;
      if(selInfo->next)
         selInfo->next->prev = selInfo->prev;
      unlockInts(flag);
      break;

   default:
      sprintf(msg, "SER%i: Invalid ioctl %i\r\n", comInf->portId, type);
      printk(msg);
      return EBADIOCTL;
   }
   return 0;
}

static void interrupt far _serialTrap(void)
{
int port, iir, lsr, msr;
serialData_t far *comInf;
selectInfo_t far *selNext;

/* clear interrupt controller */
   outp(INT_CNTL_PORT, EOI);

/* poll all interrupting devices */
   for(port=0; port<MAX_SERIALPORTS; port++)
   {
   /* is this device installed? */
      if(devTab[port] < 0)
         continue;

   /* get device data */
      comInf = (serialData_t far *) getDeviceData(devTab[port]);

   /* see if this device interrupted */
      iir = inp(COM_IIR(comInf));
      if(iir & COM_IIR_NOINT)
         continue;

   /* wake any pending tasks */
      iir &= COM_IIR_INTID;
      if(comInf->pendTask)
      {
         switch(iir)
         {
         case COM_IIR_RXDATA:
            if(comInf->pendWhy == PEND_READ)
            {
               resumeTask(comInf->pendTask, 0);
               comInf->pendTask = NULL;
            }
            break;
         case COM_IIR_TXRDY:
            if(comInf->pendWhy == PEND_WRITE)
            {
               resumeTask(comInf->pendTask, 0);
               comInf->pendTask = NULL;
            }
            break;
         }
      }

   /* wake any selecting tasks */
      for(selNext = comInf->selList; selNext != NULL; selNext = selNext->next)
      {
         switch(iir)
         {
         case COM_IIR_RXDATA:
            if(selNext->type == SEL_READ)
               resumeTask(selNext->task, devTab[port]);
            break;

         case COM_IIR_TXRDY:
            if(selNext->type == SEL_WRITE)
               resumeTask(selNext->task, devTab[port]);
            break;
         }
      }

   /* read line status */
      lsr = inp(COM_LSR(comInf));

   /* read modem status */
      msr = inp(COM_MSR(comInf));

   /* store current status */
      comInf->status = lsr | (msr << 8);
   
   /* have we got any data? */
      if(lsr & SER_RXDATA)
      {
         comInf->buffer[comInf->head++] = inp(COM_RXBUF(comInf));
         if(comInf->head >= SERIAL_BUF_SIZE)
            comInf->head = 0;
         if(comInf->head == comInf->tail)
            comInf->status |= SER_OVERRUN;
      }
   }
}

/* End */
