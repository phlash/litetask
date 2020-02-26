/*------------------------------------------------------------------------
   MOUSE.H - LiteTask mouse driver interface

   $Author:$
   $Date:$
   $Revision:$

------------------------------------------------------------------------*/

#include "select.h"

/* The maximum number of mouse devices */
#define MAX_MOUSEDEVS   1

/* ioctl()'s for mouse driver (require arg=&(int) unless otherwise stated) */
#define MSEIOCGETTYPE   0
#define MSEIOCSETTYPE   1
#define MSEIOCGETMODE   2
#define MSEIOCSETMODE   3

/* Mouse types */
#define MSE_MICROSOFT      0
#define MSE_MOUSESYSTEMS   1

/* Mouse modes */
#define MSE_DELTA_MODE     0
#define MSE_ABSOLUTE_MODE  1

/* Mouse position structure (returned by readDevice() call) */
typedef struct {
            int buttons;
            int x;
            int y;
            } mousePos_t;

/* Button state flags */
#define MSE_BUTTON_L 0x04
#define MSE_BUTTON_M 0x02
#define MSE_BUTTON_R 0x01

/* Driver data structures */
typedef struct {
            int mseType;
            int mseMode;
            int mseDev;
            taskHandle readTask;
            selectInfo_t far *selList;
            mousePos_t msePos;
            } mouseInfo_t;

/* User-callable functions */
extern int far installMouse(void);
extern int far createMouse(int ioDev, int mouseType);
extern int far deleteMouse(int devId);
extern int far removeMouse(void);

/* End */
