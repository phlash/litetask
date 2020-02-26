/*------------------------------------------------------------------------
   XLIB.C - LiteTask X11 graphics library API

   $Author:   Phlash  $
   $Date:   09 Mar 1995 20:58:34  $
   $Revision:   1.2  $

------------------------------------------------------------------------*/

#include "litetask.h"           // Kernel routines
#include "Xlib.h"               // Public interface definition
#include "Xint.h"               // Internal interface definition
#include "colors.h"             // RGB color value macros (eg: _BLACK)

/*------------------------------------------------------------------------
   Global data for X11 library.
------------------------------------------------------------------------*/

char far *X11name = "(AshbySoft *) XWindows for LiteTask, $Revision:   1.2  $";

primsDriver_t far *X11pDrv = NULL; // The primitive driver in use

static int installed = 0;          // Has the library been installed?

/*------------------------------------------------------------------------
   Public library functions
------------------------------------------------------------------------*/

int far InstallXWindows(primsDriver_t far *pDrv)
{
short flag;

/* sanity checks */
   if(!pDrv)
      return EARGS;

/* lock task and check for installed already */
   flag = lockTask();
   if(!installed)
   {
   /* set installed flag */
      installed=1;
      unlockTask(flag);

   /* set primitives driver */
      X11pDrv = pDrv;

   /* initialise the driver */
      X11pDrv->InitDriver();

   /* install hardware colormap */
      if(!XIInstallColormap())
      {
         X11pDrv->RemoveDriver();
         printk("InstallXWindows: Cannot install default colormap\r\n");
         return EINTERNAL;
      }

   /* store the display size into root window data */
      X11rootWin.w = X11pDrv->width;
      X11rootWin.h = X11pDrv->height;

   /* say hi */
      printk("XLIB: ");
      printk(X11name);
      printk("\r\nXLIB: Video driver: ");
      printk(X11pDrv->description);
      printk("\r\nXLIB: XWindows installed.\r\n");
   }   
   else
      unlockTask(flag);

/* map root window */
   XMapWindow(NULL, &X11rootWin);

/* all Done, return OK */
   return 0;
}

int far RemoveXWindows(void)
{
short flag;

   flag = lockTask();
   if(installed)
   {
   /* remove colourmap from hardware */
      XIRemoveColormap();

   /* remove graphics driver */
      X11pDrv->RemoveDriver();

   /* clear installed flag */
      installed=0;
      printk("XLIB: XWindows removed.\r\n");
   }
   unlockTask(flag);
   return 0;
}

/*------------------------------------------------------------------------
   X11 Interface functions
------------------------------------------------------------------------*/

Display far * far XOpenDisplay(char far *display)
{
Display far *newDisplay;

/* allocate a display structure as a resource tag and return it */
   newDisplay = (Display far *)malloc( sizeof(Display) );
   if(!newDisplay)
      printk("XOpenDisplay: out of memory\r\n");
   else
   {
      newDisplay->name  = X11name;
      newDisplay->width = X11pDrv->width;
      newDisplay->height = X11pDrv->height;
      newDisplay->wMM = _WIDTH_MM;
      newDisplay->hMM = _HEIGHT_MM;
      newDisplay->planes = X11pDrv->planes;
      newDisplay->black_pixel = _BLACK_PIX;
      newDisplay->white_pixel = _WHITE_PIX;
      newDisplay->colormap = XIDefaultColormap();
      newDisplay->gc = XIDefaultGC();
      newDisplay->root_window = (Window)&X11rootWin;
      newDisplay->resources.windows = NULL;
      newDisplay->resources.gcs = NULL;
      newDisplay->resources.cmaps = NULL;
      newDisplay->head = newDisplay->tail = 0;
      newDisplay->pending = NULL;
   }
   return newDisplay;
}


void far XCloseDisplay(Display far *dpy)
{
void far *nextRes, far *tmp;

/* Remove all resources tagged onto the display structure */
/* 0: Windows */
   for(nextRes = dpy->resources.windows; nextRes; nextRes = tmp)
   {
      tmp = ((wininfo_t far *)nextRes)->next;
      XDestroyWindow(dpy, nextRes);
   }

/* 1: Graphics Contexts */
   for(nextRes = dpy->resources.gcs; nextRes; nextRes = tmp)
   {
      tmp = ((gc_t far *)nextRes)->next;
      free(nextRes);
   }

/* 2: Colormaps */
   for(nextRes = dpy->resources.cmaps; nextRes; nextRes = tmp)
   {
      tmp = ((colormap_t far *)nextRes)->next;
      free( ((colormap_t far *)nextRes)->map );
      free( ((colormap_t far *)nextRes)->flags );
      free(nextRes);
   }
}


void far XSync(Display far *dpy, int discard)
{
/* very little! */
}


void far XFlush(Display far *dpy)
{
/* Not a lot! */
}

/* End */
