/*------------------------------------------------------------------------
   BIOSCON.H - LiteTask BIOS level console device driver

   $Author:   Phlash  $
   $Date:   04 Jun 1994 20:28:40  $
   $Revision:   1.3  $

------------------------------------------------------------------------*/

#include "select.h"

/* Maximum number of consoles (BIOS video pages) */
#define MAX_CONSOLES    8

/* normal screen dimensions */
#define SCREEN_WIDTH    80
#define SCREEN_HEIGHT   25
#define DEFAULT_TABSIZE 8

/* driver mode bits */
#define BC_ECHO         1
#define BC_BLK          2
#define BC_COOKED       4

/* available ioctl() calls */

/* These control echoing of characters typed in, set arg=NULL */
#define BCIOCECHO        0
#define BCIOCNOECHO      1

/* These set/clear cooked mode (CR/LF translation) */
#define BCIOCCOOKED      2
#define BCIOCRAW         3

/* These allow get/set of the driver's internal bitfield,
   and require arg=&(int mode) */
#define BCIOCGETMODE     4
#define BCIOCSETMODE     5

/* Driver data structures */
#define KEY_BUF_SIZE     16
typedef struct {
            int page;
            int mode;
            int cx, cy;
            int head;
            int tail;
            int keyBuf[KEY_BUF_SIZE];
            taskHandle pendTask;
            selectInfo_t far *selList;
            } consoleData_t;

/* User-callable installation routine */
extern int far installConsole(void);
extern int far createConsole(int BIOSPage);
extern int far deleteConsole(int devId);
extern int far removeConsole(void);

/* End */
