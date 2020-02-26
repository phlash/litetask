/*------------------------------------------------------------------------
   XROOT.C - LiteTask X11 library default root window handler

   $Author:$
   $Date:$
   $Revision:$

------------------------------------------------------------------------*/
#include "litetask.h"
#include "Xlib.h"
#include "Xint.h"
#include "colors.h"

/* Constants */
#define RLE_NAME "c:\\phil\\litetask\\x11\\xlib\\AshbySo2.rle"

/* Globals */
static WORD far *rootPixmap=NULL;
static BYTE rootBitmap[] = {
8, 8, 0x0C, 0x36, 0x63, 0xC0, 0x81, 0x63, 0x36, 0x18
};

/* Types */

/* Code */
static WORD far *_LoadPixmap(char far *name)
{
WORD w, h, l, far *pixmap;
int fd;

/* Open the RLE file */
   if((fd = open(name, O_RDONLY)) < 0)
      return NULL;

/* read the pixmap dimensions & file length */
   read(fd, &w, sizeof(WORD));
   read(fd, &h, sizeof(WORD));
   read(fd, &l, sizeof(WORD));

/* allocate storage */
   pixmap=(WORD far *)malloc((l+3)*sizeof(WORD));
   if(!pixmap)
   {
      close(fd);
      return NULL;
   }
   pixmap[0] = w;
   pixmap[1] = h;
   pixmap[2] = l;

/* read pixmap into memory */
   read(fd, &pixmap[3], l*sizeof(WORD));
   close(fd);
   return pixmap;
}

static void _RootPixmap(int x, int y, int w, int h)
{
int i, idx;
rectangle_t pixRect, expRect;

/* load the pixmap (if not done already) */
   if(!rootPixmap)
      if((rootPixmap=_LoadPixmap(RLE_NAME)) == NULL)
         return;

/* do we need to draw anything? */
   pixRect.w = rootPixmap[0];
   pixRect.h = rootPixmap[1];
   pixRect.x = (X11pDrv->width-pixRect.w)/2;
   pixRect.y = (X11pDrv->height-pixRect.h)/2;
   expRect.x = x;
   expRect.y = y;
   expRect.w = w;
   expRect.h = h;
   if(!XIRectanglesOverlap(&pixRect, &expRect))
      return;

   i = 0;
   idx = 3;
   while(i < pixRect.w * pixRect.h)
   {
      X11pDrv->DrawLine(pixRect.x+i%pixRect.w,
               pixRect.y+pixRect.h-(i/pixRect.w),
               pixRect.x+(i%pixRect.w)+rootPixmap[idx],
               pixRect.y+pixRect.h-(i/pixRect.w),
               (long)rootPixmap[idx+1], AllPlanes);
      i+=rootPixmap[idx];
      idx+=2;
      if(idx > rootPixmap[2]+3)
         break;
   }
}

static void _RootExpose(int x, int y, int w, int h)
{
int bw, bh, sx, sy, ex, ey;

/* round x,y,w,h to nearest multiple of bitmap size */
   bw = (int)rootBitmap[0];
   bh = (int)rootBitmap[1];
   ex = (x+w+7)/bw*bw;
   ey = (y+h+7)/bh*bh;
   sx = x/bw*bw;
   sy = y/bh*bh;

/* render the rectangle in BLACK, then add the Bitmap */
   X11pDrv->FillRectangle(sx, sy, ex-1, ey-1, (long)_BLACK_PIX, AllPlanes);
   for(y=sy; y<ey; y+=bh)
   {
      for(x=sx; x<ex; x+=bw)
      {
         X11pDrv->DrawBitmap(x, y, bw, bh, &rootBitmap[2],
            (long)_GRAY_PIX, 255L);
      }
   }

/* Finally add centered pixmap */
   _RootPixmap(sx, sy, ex-sx, ey-sy);
}

int far XIRootEvent(XEvent far *event)
{
   switch(event->type)
   {
   case Expose:
      _RootExpose(event->xexpose.x, event->xexpose.y,
         event->xexpose.width, event->xexpose.height);
      break;
   default:
      printk("_RootWin: Invalid event recieved\r\n");
      return False;
   }
   return True;
}

/* End */
