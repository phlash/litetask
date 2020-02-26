/*------------------------------------------------------------------------
   IOSYS.C - I/O System for LiteTask Kernel

   $Author:   Phlash  $
   $Date:   12 May 1994 19:53:02  $
   $Revision:   1.6  $

------------------------------------------------------------------------*/

#include "litetask.h"

/* Maximum number of installed devices */
#define MAX_DEVS    128

/* global table of device data structures */
static driverInfo_t devices[MAX_DEVS] = { 0 };

/* ****** Driver Functions ****** */

/*
 * newIOSysDevice() - Adds a new device to the IO System
 */
int far newIOSysDevice(int index, driverInfo_t far *newDevice)
{
short flag;

/* sanity checks */
   if(index >= MAX_DEVS)
      return EDEVICEID;
   if((newDevice->type != CHR_DEV) && (newDevice->type != BLK_DEV))
      return EBADDEV;

/* insert the new device into the array */
   flag = lockTask();
   if(index >= 0)
   {
      if(devices[index].type != FREE_DEV)
      {
         unlockTask(flag);
         return EDEVUSED;
      }
   }
   else
   {
      for(index = 0; index < MAX_DEVS; index++)
      {
         if(devices[index].type == FREE_DEV)
            break;
      }
      if(index >= MAX_DEVS)
      {
         unlockTask(flag);
         return ENODEVSPACE;
      }
   }
   devices[index] = *newDevice;
   unlockTask(flag);
   return index;
}

/*
 * deleteIOSysDevice() - removes a device from the IO System
 */
int far deleteIOSysDevice(int device)
{
short flag;

/* sanity check, then clear array entry */
   if(device < 0 || device >= MAX_DEVS)
      return EDEVICEID;

   flag = lockTask();
   devices[device].type = FREE_DEV;
   unlockTask(flag);
   return 0;
}

/* ********* Client functions *********** */

/*
 * openDevice() - Open a device through the IO System
 */

int far openDevice(int device, long timeout)
{
int rVal;

/* Sanity checks */
   if(device < 0 || device >= MAX_DEVS)
      return EDEVICEID;
   if((devices[device].type != CHR_DEV) && (devices[device].type != BLK_DEV))
      return EBADDEV;

/* now check for open() function and call it */
   if(devices[device].open)
      rVal = (*(devices[device].open))(devices[device].drvInfo, timeout);
   else
      rVal = 0;
   
   return rVal;
}

/*
 * closeDevice() - close a device through the IO System
 */

int far closeDevice(int device)
{
int rVal;

/* Sanity checks */
   if(device < 0 || device >= MAX_DEVS)
      return EDEVICEID;
   if((devices[device].type != CHR_DEV) && (devices[device].type != BLK_DEV))
      return EBADDEV;

/* now check for close() function and call it */
   if(devices[device].close)
      rVal = (*(devices[device].close))(devices[device].drvInfo);
   else
      rVal = 0;
   
   return rVal;
}

/*
 * readDevice() - Read data from a device through the IO System
 */

int far readDevice(int device, void far *buff, int size, long offset)
{
int rVal;

/* Sanity checks */
   if(device < 0 || device >= MAX_DEVS)
      return EDEVICEID;
   if((devices[device].type != CHR_DEV) && (devices[device].type != BLK_DEV))
      return EBADDEV;

/* now check for read() function and call it */
   if(devices[device].read)
      rVal = (*(devices[device].read))(devices[device].drvInfo, buff, size, offset);
   else
      rVal = 0;
   
   return rVal;
}

/*
 * writeDevice() - Write data to a device through the IO System
 */

int far writeDevice(int device, void far *buff, int size, long offset)
{
int rVal;

/* Sanity checks */
   if(device < 0 || device >= MAX_DEVS)
      return EDEVICEID;
   if((devices[device].type != CHR_DEV) && (devices[device].type != BLK_DEV))
      return EBADDEV;

/* now check for write() function and call it */
   if(devices[device].write)
      rVal = (*(devices[device].write))(devices[device].drvInfo, buff, size, offset);
   else
      rVal = 0;
   
   return rVal;
}

/*
 * ioctlDevice() - Perform IO ConTroL on a device through the IO System
 */

int far ioctlDevice(int device, int type, void far *buff)
{
int rVal;

/* Sanity checks */
   if(device < 0 || device >= MAX_DEVS)
      return EDEVICEID;
   if((devices[device].type != CHR_DEV) && (devices[device].type != BLK_DEV))
      return EBADDEV;

/* now check for ioctl() function and call it */
   if(devices[device].ioctl)
      rVal = (*(devices[device].ioctl))(devices[device].drvInfo, type, buff);
   else
      rVal = EBADIOCTL;
   
   return rVal;
}

/*
 * getDeviceType() - Reads the type of the specified device
 */

int far getDeviceType(int device)
{
/* Sanity checks */
   if(device < 0 || device >= MAX_DEVS)
      return EDEVICEID;

/* return type */
   return devices[device].type;
}

/*
 * getDeviceData() - reads the drvInfo pointer of the specified device
 */

void far * far getDeviceData(int device)
{
/* Sanity checks */
   if(device < 0 || device >= MAX_DEVS)
      return EDEVICEID;
   if(devices[device].type == FREE_DEV)
      return EBADDEV;

/* return pointer */
   return devices[device].drvInfo;
}

/* End */
