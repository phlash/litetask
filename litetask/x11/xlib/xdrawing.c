/*------------------------------------------------------------------------
   XDRAWING.C - LiteTask X11 library drawing / GC routines

   $Author:$
   $Date:$
   $Revision:$

------------------------------------------------------------------------*/
#include "litetask.h"
#include "Xlib.h"
#include "Xint.h"
#include "colors.h"

static gc_t _def_gc =           // Default Graphics Context (GC)
   { _LIGHTGRAY_PIX,               // forground color
     _BLACK_PIX,                   // background color
     AllPlanes,                    // drawing plane mask
     NULL };                       // Chain pointer

/* ********** Private routines ************ */
GC far XIDefaultGC(void)
{
   return (GC)&_def_gc;
}

/* ********** Graphics Context routines ********** */

GC far XCreateGC(Display far *dpy, Drawable d, unsigned long mask, void far *data)
{
gc_t far *newGC;

/* allocate a new GC */
   if((newGC = (gc_t far *)malloc(sizeof(gc_t))) == NULL)
   {
      printk("XCreateGC: out of memory\r\n");
      return (GC)NULL;
   }
   *newGC = _def_gc;
   newGC->next = (gc_t far *)dpy->resources.gcs;
   dpy->resources.gcs = (GC)newGC;

/* Should update from supplied data here (XGCValues structure?) */

   return (GC)newGC;
}

int far XSetForeground(Display far *dpy, GC gc, unsigned long fg)
{
   ((gc_t far *)gc)->fg = fg;
   return True;
}

int far XSetPlaneMask(Display far *dpy, GC gc, unsigned long planemask)
{
   ((gc_t far *)gc)->pmask = planemask;
   return True;
}

/* ********** Drawing routines *********** */

int far XDrawPoint(Display far *dpy, Drawable d, GC gc, int x, int y)
{
wininfo_t far *w = (wininfo_t far *)d;
gc_t far *g = (gc_t far *)gc;

/* Draw point relative to Window */   
   X11pDrv->DrawPoint(w->x + x, w->y + y, g->fg, g->pmask);
   return True;
}

int far XDrawLine(Display far *dpy, Drawable d, GC gc, int x1, int y1, int x2, int y2)
{
wininfo_t far *win = (wininfo_t far *)d;
gc_t far *g = (gc_t far *)gc;

/* Draw line relative to Window */   
   X11pDrv->DrawLine(win->x+x1, win->y+y1, win->x+x2, win->y+y2,
      g->fg, g->pmask);
   return True;
}

int far XDrawLines(Display far *dpy, Drawable d, GC gc, XPoint far *pnts, int npoints, int mode)
{
wininfo_t far *w = (wininfo_t far *)d;
gc_t far *g = (gc_t far *)gc;
int i,cx,cy;
char buf[80];

/* check args */
   if(npoints < 2)
   { 
      sprintf(buf, "XDrawLines: invalid args: npoints=%i\r\n", npoints);
      printk(buf);
      return False;
   }

/* draw lines from point to point */
   cx = pnts[0].x;
   cy = pnts[0].y;
   for(i=1; i<npoints; i++)
   {
      if(mode == CoordModeOrigin)
      {
         X11pDrv->DrawLine(w->x+cx, w->y+cy,
            w->x+pnts[i].x, w->y+pnts[i].y,
            g->fg, g->pmask);
         cx = pnts[i].x;
         cy = pnts[i].y;
      }
      else
      {
         X11pDrv->DrawLine(w->x+cx, w->y+cy,
            w->x+cx+pnts[i].x, w->y+cy+pnts[i].y,
            g->fg, g->pmask);
         cx += pnts[i].x;
         cy += pnts[i].y;
      }
   }
   return True;
}

int far XDrawRectangle(Display far *dpy, Drawable d, GC gc, int x, int y, int w, int h)
{
wininfo_t far *win = (wininfo_t far *)d;
gc_t far *g = (gc_t far *)gc;

/* Render rectangle in client area using Primitive */
   X11pDrv->DrawRectangle(win->x+x, win->y+y,
                  win->x+x+w, win->y+y+h, g->fg, g->pmask);
   return True;
}

int far XFillRectangle(Display far *dpy, Drawable d, GC gc, int x, int y, int w, int h)
{
wininfo_t far *win = (wininfo_t far *)d;
gc_t far *g = (gc_t far *)gc;

/* Render rectangle in client area using Primitive */
   X11pDrv->FillRectangle(win->x+x, win->y+y,
               win->x+x+w, win->y+y+h, g->fg, g->pmask);
   return True;
}

int far XFillPolygon(Display far *dpy, Drawable d, GC gc, XPoint far *pnts, int npoints, int shape, int mode)
{
wininfo_t far *w = (wininfo_t far *)d;
gc_t far *g = (gc_t far *)gc;
int i,cx,cy,x1,y1,x2,y2;

/* draw lines from point to point (then close path) */
   x1 = x2 = cx = pnts[0].x;
   y1 = y2 = cy = pnts[0].y;
   for(i=1; i<npoints; i++)
   {
      if(mode == CoordModeOrigin)
      {
         X11pDrv->DrawLine(w->x+cx, w->y+cy,
            w->x+pnts[i].x, w->y+pnts[i].y,
            g->fg, g->pmask);
         cx = pnts[i].x;
         cy = pnts[i].y;
      }
      else
      {
         X11pDrv->DrawLine(w->x+cx, w->y+cy,
            w->x+cx+pnts[i].x, w->y+cy+pnts[i].y,
            g->fg, g->pmask);
         cx += pnts[i].x;
         cy += pnts[i].y;
      }
   /* record maximum extent of shape */
      if(cx < x1) x1 = cx;
      if(cx > x2) x2 = cx;
      if(cy < y1) y1 = cy;
      if(cy > y2) y2 = cy;
   }
   X11pDrv->DrawLine(w->x+cx, w->y+cy,
      w->x+pnts[0].x, w->y+pnts[0].y,
      g->fg, g->pmask);

/*
   scan rectangle formed by maximum extent, fill pixels between borders: 
   this implements the Odd/Even fill rule
*/
#ifdef SLOWFILL
   i = 0;
   for(cy=y1; cy<y2; cy++)
   {
      for(cx=x1; cx<x2; cx++)
      {
         if(X11pDrv->GetPixel(w->x+cx, w->y+cy, g->pmask) == (g->fg & g->pmask))
            i = ~i;
         if(i)
            X11pDrv->DrawPoint(w->x+cx, w->y+cy, g->fg, g->pmask);
      }
   }
#endif
   return True;
}

int far XDrawString(Display far *dpy, Drawable d, GC gc, int x, int y, char far *str, int len)
{
wininfo_t far *w = (wininfo_t far *)d;
gc_t far *g = (gc_t far *)gc;

/* render in client area using Primitive */
   X11pDrv->DrawString(w->x+x, w->y+y, str, len, g->fg, g->pmask);
   return True;
}

int far XCopyArea(Display far *dpy, Drawable src, Drawable dest, GC gc,
              int srcx, int srcy, int w, int h, int destx, int desty)
{
/* Can't do this I'm afraid! */
   return False;
}

Pixmap far XCreatePixmap(Display far *dpy, Drawable d, int w, int h, int depth)
{
/* Can't do this I'm afraid! */
   return (Pixmap)NULL;
}

int far XFreePixmap(Display far *dpy, Pixmap p)
{
/* Not much! */
   return True;
}

/* End */
