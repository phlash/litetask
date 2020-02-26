/*------------------------------------------------------------------------
   IOCTL.H - LiteTask standard ioctl()'s for all device drivers

   $Author:   Phlash  $
   $Date:   28 Apr 1994 19:17:34  $
   $Revision:   1.1  $

------------------------------------------------------------------------*/

/* Unless otherwise stated, all ioctl()'s require arg=NULL */

/* Start at far end of number range */
#define STDIOCBASE      0xF000

/* Blocking / non-blocking */
#define STDIOCBLK       (STDIOCBASE|0)
#define STDIOCNBLK      (STDIOCBASE|1)

/* Asyncronous IO */
#define STDIOCASYNC     (STDIOCBASE|2)
#define STDIOCNASYNC    (STDIOCBASE|3)

/* Bytes in IO queues, require arg=&(long) */
#define STDIOCNREAD     (STDIOCBASE|4)
#define STDIOCNWRITE    (STDIOCBASE|5)

/* Select on device, requires arg=&(selectInfo_t) */
#define STDIOCSELECT    (STDIOCBASE|6)
#define STDIOCNSELECT   (STDIOCBASE|7)

/* End */
