/*------------------------------------------------------------------------
   BIOSDISK.H - LiteTask BIOS level disk driver

   $Author:   Phlash  $
   $Date:   25 May 1994 20:16:46  $
   $Revision:   1.1  $

------------------------------------------------------------------------*/

/* Maximum number of BIOS supported disk drives */
#define MAX_BIOSDISKS   4

/* ioctl()'s */
/* These get/set the disk geometry, and require arg=&(driveData_t) */
#define BDIOCGETGEOM    0
#define BDIOCSETGEOM    1

/* These get/set the device timeout, and require arg=&(int) */
#define BDIOCGETTIMEOUT 2
#define BDIOCSETTIMEOUT 3

/* This formats a disk, and is not currently implemented :) */
#define BDIOCFORMAT     100

/* Driver data structures */
typedef struct {
            int  drive;
            WORD cyls;
            WORD heads;
            WORD sectors;
            } driveData_t;

/* User-callable driver routines */
extern int far installBIOSDisk(void);
extern int far createBIOSDisk(int BIOSDrive);
extern int far deleteBIOSDisk(int devId);
extern int far removeBIOSDisk(void);

/* End */
