/*------------------------------------------------------------------------
   MOUSE.C - LiteTask mouse driver

   $Author:   Phlash  $
   $Date:   03 Jun 1994 20:09:18  $
   $Revision:   1.0  $

------------------------------------------------------------------------*/

#include "litetask.h"
#include "serial.h"
#include "ioctl.h"
#include "mouse.h"

static char *pvcsId = "$Revision:   1.0  $\r\n";

/* Constants */
#define MSE_TIMEOUT  18L

/* Prototypes */
static int far read(void far *drvInf, char far *buf, int size, long offset);
static int far ioctl(void far *drvInf, int ioctl, void far *buff);
static int far _mouseReader(mouseInfo_t far *mouseInf, int devIdx);
static void near decodeMicrosoft(mousePos_t far *msePos, char far *mseData, int mseMode);
static void near decodeMouseSys(mousePos_t far *msePos, char far *mseData, int mseMode);

/* Globals */
static int installed = 0;
static int devTab[MAX_MOUSEDEVS];

/*------------------------------------------------------------------------
   This driver decodes Microsoft or MouseSystems mice protocols,
   read via the device specified in the createMouse() call (usually a
   serial port). It always passes back a mouse position & status packet
   on read requests, this is defined in MOUSE.H. Write requests are not
   supported. Ioctl's allow an application to select() for updates to the
   mouse position or status, or change the mouse type / mode in use.
------------------------------------------------------------------------*/

/* Install / remove driver */
int far installMouse(void)
{
short flag;
int i;

/* Say hi! */
   flag = lockTask();
   if(!installed)
   {
      for(i=0; i<MAX_MOUSEDEVS; i++)
         devTab[i] = -1;
      printk("MSE: LiteTask mouse driver, (c) (AshbySoft *) 1994: ");
      printk(pvcsId);
   }
   unlockTask(flag);
   return 0;
}

int far removeMouse(void)
{
short flag;
int i;

/* Delete all instances of the driver */
   flag = lockTask();
   for(i=0; i<MAX_MOUSEDEVS; i++)
   {
      if(devTab[i] >= 0)
         deleteMouse(devTab[i]);
   }
   unlockTask(flag);
   printk("MSE: Driver removed.\r\n");
   return 0;
}

/* Create/delete instances */
int far createMouse(int device, int mouseType)
{
mouseInfo_t far *newMouse;
driverInfo_t drvInfo;
int i, devIdx = -1;
short flag;
char far *strType, msg[80];

/* sanity checks */
   if(device < 0)
      return EARGS;
   if(mouseType != MSE_MICROSOFT && mouseType != MSE_MOUSESYSTEMS)
      return EARGS;

   if(openDevice(device, MSE_TIMEOUT) < 0)
   {
      sprintf(msg, "MSE: Error opening device %i\r\n", device);
      printk(msg);
      return EBADDEV;
   }
   closeDevice(device);

   flag = lockTask();
   for(i=0; i<MAX_MOUSEDEVS; i++)
   {
      if(devTab[i] >= 0)
      {
         newMouse = (mouseInfo_t far *)getDeviceData(devTab[i]);
         if(device == newMouse->mseDev)
         {
            unlockTask(flag);
            sprintf(msg, "MSE: Attempt to re-create mouse on device %i\r\n",
               device);
            printk(msg);
            return EARGS;
         }
      }
      else
         if(devIdx < 0)
            devIdx = i;
   }
   if(devIdx < 0)
   {
      unlockTask(flag);
      printk("MSE: Attempt to create too many mouse devices\r\n");
      return ENODEVSPACE;
   }

/* allocate device control block */
   newMouse = (mouseInfo_t far *)malloc( sizeof(mouseInfo_t) );
   if(!newMouse)
   {
      unlockTask(flag);
      printk("MSE: Out of memory\r\n");
      return EINTERNAL;
   }

/* fill out control block and start read task */
   newMouse->mseType = mouseType;
   newMouse->mseMode = MSE_DELTA_MODE;
   newMouse->mseDev = device;
   newMouse->selList = NULL;
   newMouse->msePos.x = 0;
   newMouse->msePos.y = 0;
   newMouse->msePos.buttons = 0;
   newMouse->readTask =
      newTask(MINSTACK, _mouseReader,
         sizeof(void far *) + sizeof(int), newMouse, devIdx);
   if(!newMouse->readTask)
   {
      unlockTask(flag);
      sprintf(msg, "MSE%i: Cannot create read task\r\n", device);
      printk(msg);
      free(newMouse);
      return EINTERNAL;
   }

/* install in device table */
   drvInfo.type  = CHR_DEV;
   drvInfo.drvInfo=newMouse;
   drvInfo.open  = NULL;
   drvInfo.close = NULL;
   drvInfo.read  = read;
   drvInfo.write = NULL;
   drvInfo.ioctl = ioctl;
   devTab[devIdx] = newIOSysDevice(-1, &drvInfo);
   if(devTab[devIdx] < 0)
   {
      unlockTask(flag);
      sprintf(msg, "MSE%i: Cannot create device table entry\r\n", device);
      printk(msg);
      free(newMouse);
      return EINTERNAL;
   }
   unlockTask(flag);

/* Done! */
   if(mouseType == MSE_MICROSOFT)
      strType = "Microsoft";
   else
      strType = "Mouse Systems";
   sprintf(msg, "MSE%i: Installed as device %i, type %s\r\n", device,
      devTab[devIdx],
      strType);
   printk(msg);
   return devTab[devIdx];
}

int far deleteMouse(int device)
{
mouseInfo_t far *oldMouse;
taskHandle tmpHandle;
int i;
short flag;
char msg[40];

/* Search table for specified device */
   flag = lockTask();
   for(i=0; i<MAX_MOUSEDEVS; i++)
   {
      if(device == devTab[i])
         break;
   }
   if(i >= MAX_MOUSEDEVS)
   {
      unlockTask(flag);
      return EARGS;
   }

/* clear read task handle, then wait for read task to terminate */
   oldMouse = (mouseInfo_t far *)getDeviceData(device);
   tmpHandle = oldMouse->readTask;
   oldMouse->readTask = NULL;
   sprintf(msg, "MSE%i: Please move the mouse..", oldMouse->mseDev);
   printk(msg);
   unlockTask(flag);
   waitTask(tmpHandle, NOTIMEOUT);

/* remove from device table */
   deleteIOSysDevice(device);

/* free control block, and return */
   sprintf(msg, "MSE%i: Device deleted\r\n", oldMouse->mseDev);
   printk(msg);
   free(oldMouse);
   return 0;
}

static int far read(mouseInfo_t far *mseInf, char far *buf, int size, long offset)
{
mousePos_t far *msePos=(mousePos_t far *)buf;
short flag;

/* check size is big enough */
   if(size < sizeof(mousePos_t))
      return EARGS;

/* write info to buffer */
   flag = lockTask();
   *msePos = (mseInf->msePos);
   unlockTask(flag);
   return sizeof(mousePos_t);
}

static int far ioctl(mouseInfo_t far *mseInf, int ioctl, void far *buff)
{
int tmp;
selectInfo_t far *selInf;
short flag;
char msg[40];

/* see what we need to do */
   switch(ioctl)
   {
   case MSEIOCGETTYPE:
      *((int far *)buff) = mseInf->mseType;
      break;

   case MSEIOCSETTYPE:
      tmp = *((int far *)buff);
      if(tmp != MSE_MICROSOFT && tmp != MSE_MOUSESYSTEMS)
         return EARGS;
      mseInf->mseType = tmp;
      break;

   case MSEIOCGETMODE:
      *((int far *)buff) = mseInf->mseMode;
      break;

   case MSEIOCSETMODE:
      tmp = *((int far *)buff);
      if(tmp != MSE_DELTA_MODE && tmp != MSE_ABSOLUTE_MODE)
         return EARGS;
      mseInf->mseMode = tmp;
      break;

   case STDIOCNREAD:
      *((long far *)buff) = 0L;        // NB: This forces select to block...
      break;

   case STDIOCSELECT:
      flag = lockTask();
      selInf = (selectInfo_t far *)buff;
      selInf->next = mseInf->selList;
      selInf->prev = NULL;
      mseInf->selList = selInf;
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
         mseInf->selList = selInf->next;
      unlockTask(flag);
      break;

   default:
      sprintf(msg, "MSE%i: Invalid ioctl %i\r\n", mseInf->mseDev, ioctl);
      printk(msg);
      return EBADIOCTL;
   }
   return 0;
}

static int far _mouseReader(mouseInfo_t far *mseInf, int devIdx)
{
int inSync = 0;
int dataPtr = 0;
int byte, syncMask, syncTest, dataSize;
void (near *decode)(mousePos_t far *, char far *, int);
selectInfo_t far *selInf;
short flag;
char mseData[5], msg[80];

/* open device (blocking mode) */
   if(openDevice(mseInf->mseDev, MSE_TIMEOUT) < 0)
   {
      sprintf(msg, "MSE%i: Cannot open IO device\r\n", mseInf->mseDev);
      printk(msg);
      return EINTERNAL;
   }
   if(ioctlDevice(mseInf->mseDev, STDIOCBLK, NULL) < 0)
   {
      sprintf(msg, "MSE%i: Cannot set device to blocking mode\r\n",
         mseInf->mseDev);
      printk(msg);
      return EINTERNAL;
   }

/* read loop */
   for(;;)
   {
   /* check our task handle, if invalid then we have been asked to terminate */
      if(!mseInf->readTask)
      {
         closeDevice(mseInf->mseDev);
         return 0;
      }

   /* read a byte from the device */
      byte = 0;
      if(readDevice(mseInf->mseDev, &byte, 1, 0L) != 1)
      {
         sprintf(msg, "MSE%i: Error reading device\r\n", mseInf->mseDev);
         printk(msg);
         break;
      }

   /* choose protocol settings */
      switch(mseInf->mseType)
      {
      case MSE_MICROSOFT:
         syncMask = 0xC0;
         syncTest = 0xC0;
         dataSize = 3;
         decode = decodeMicrosoft;
         break;

      case MSE_MOUSESYSTEMS:
         syncMask = 0xF0;
         syncTest = 0x80;
         dataSize = 5;
         decode = decodeMouseSys;
         break;

      default:
         sprintf(msg, "MSE%i: Mouse type invalid\r\n", mseInf->mseDev);
         printk(msg);
         continue;
      }
   
   /* get in sync */
      if(!inSync)
      {
         dataPtr = 0;
         if((byte & syncMask) == syncTest)
            inSync = 1;
         else
            continue;
      }

   /* check we haven't lost sync */
      if(!dataPtr && (byte & syncMask) != syncTest)
      {
         sprintf(msg, "MSE%i: Mouse protocol sync lost\r\n", mseInf->mseDev);
         printk(msg);
         inSync = 0;
         continue;
      }

   /* add byte to message, decode a packet if we've got one */
      mseData[dataPtr++] = (char)byte;
      if(dataPtr >= dataSize)
      {
         dataPtr = 0;
         flag = lockTask();
         (*decode)(&mseInf->msePos, mseData, mseInf->mseMode);
         if(mseInf->selList)
         {
            for(selInf = mseInf->selList; selInf; selInf = selInf->next)
               resumeTask(selInf->task, devTab[devIdx]);
         }
         unlockTask(flag);
      }
   }

/* all errors bomb out of the loop to here */
   closeDevice(mseInf->mseDev);
   return EINTERNAL;
}

static void near decodeMicrosoft(mousePos_t far *msePos, char far *mseData, int mseMode)
{
/* Decode the Microsoft mouse protocol */
   msePos->buttons = 0;
   if(mseData[0] & 0x20)
      msePos->buttons |= MSE_BUTTON_L;
   else
      msePos->buttons &= ~MSE_BUTTON_L;
   if(mseData[0] & 0x10)
      msePos->buttons |= MSE_BUTTON_R;
   else
      msePos->buttons &= ~MSE_BUTTON_R;
   if(mseData[1] & 0x20)
      mseData[1] |= 0xC0;
   else
      mseData[1] &= 0x7F;
   if(mseData[2] & 0x20)
      mseData[2] |= 0xC0;
   else
      mseData[2] &= 0x7F;
   switch(mseMode)
   {
   case MSE_DELTA_MODE:
      msePos->x = (int)mseData[1];
      msePos->y = (int)mseData[2];
      break;
   case MSE_ABSOLUTE_MODE:
      msePos->x += (int)mseData[1];
      msePos->y += (int)mseData[2];
      break;
   }
}

static void near decodeMouseSys(mousePos_t far *msePos, char far *mseData, int mseMode)
{
/* Decode the Mouse Systems protocol */
   msePos->buttons = 0;
   if(mseData[0] & 0x04)
      msePos->buttons &= ~MSE_BUTTON_L;
   else
      msePos->buttons |= MSE_BUTTON_L;
   if(mseData[0] & 0x02)
      msePos->buttons &= ~MSE_BUTTON_M;
   else
      msePos->buttons |= MSE_BUTTON_M;
   if(mseData[0] & 0x01)
      msePos->buttons &= ~MSE_BUTTON_R;
   else
      msePos->buttons |= MSE_BUTTON_R;
   switch(mseMode)
   {
   case MSE_DELTA_MODE:
      msePos->x = (int)mseData[1];
      msePos->y = -(int)mseData[2];
      break;
   case MSE_ABSOLUTE_MODE:
      msePos->x += (int)mseData[1];
      msePos->y += -(int)mseData[2];
      break;
   }
}

/* End */
