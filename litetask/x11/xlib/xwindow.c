/*------------------------------------------------------------------------
   XWINDOW.C - LiteTask X11 graphics library, Window routines

   $Author:$
   $Date:$
   $Revision:$

------------------------------------------------------------------------*/

#include "litetask.h"           // Kernel routines
#include "Xlib.h"               // Public interface definition
#include "Xint.h"               // Internal interface definition
#include "colors.h"             // VGA colour macros

/*------------------------------------------------------------------------
   Private data
------------------------------------------------------------------------*/

wininfo_t X11rootWin =             // Root Window
   { 0, 0, 0, 0,                   // x, y, width & height *
   0,                              // border width
   _WHITE_PIX, _BLACK_PIX,         // border and background colors
   (int)ExposureMask,              // Expose events allowed
   NULL, NULL, NULL,               // no owner or windows in owner chain
   NULL, NULL, NULL,               // no parent/children/siblings (yet)
   NULL, NULL                      // no display hierarchy (yet)
   };

static wininfo_t far *topWin=&X11rootWin; // The top of the display stack

/*------------------------------------------------------------------------
   Private library functions
------------------------------------------------------------------------*/

int far XIChainDisplayEvent(wininfo_t far *start, XEvent far *event, int dir)
{
wininfo_t far *np;

/* trace display hierarchy from start, queue event for each window which
   is in the event area & expects such an event */
   for(np=start; np; np = (dir == DIR_UP) ? np->up : np->down)
   {
      if(np->event_mask & event->type)
      {
         if(PointInWindow(event->xany.x, event->xany.y, np) ||
            PointInWindow(event->xany.x+event->xany.width, event->xany.y, np) ||
            PointInWindow(event->xany.x, event->xany.y+event->xany.height, np) ||
            PointInWindow(event->xany.x+event->xany.width, event->xany.y+event->xany.height, np))
         {
            XIQueueEvent(np, event);
         }
      }
   }
   return True;
}

void far XIDumpOwnershipChain(Display far *dpy)
{
wininfo_t far *w = (wininfo_t far *)dpy->resources.windows;
char buf[80];
short flag;

   flag = lockTask();
   printk("->XIDumpOwnershipChain\r\n");
   while(w)
   {
      sprintf(buf, "w = %x:%x, w->next = %x:%x, w->prev = %x:%x\r\n",
         FP_SEG(w), FP_OFF(w),
         FP_SEG(w->next), FP_OFF(w->next),
         FP_SEG(w->prev), FP_OFF(w->prev) );
      printk(buf);
      w = w->next;
   }
   printk("<-XIDumpOwnershipChain\r\n");
   unlockTask(flag);
}

void far XIDumpAncestryChain(wininfo_t far *start, int indent)
{
wininfo_t far *w;
int i;
short flag;

static char buf[80];

   flag = lockTask();
   if(!indent)
      printk("->XIDumpAncestryChain\r\n");
   for(w = start; w; w = w->siblings)
   {
      for(i=0; i<indent; i++)
         printk(" ");
      sprintf(buf, "w = %x:%x, w->p = %x:%x, w->c = %x:%x, w->s = %x:%x\r\n",
         FP_SEG(w), FP_OFF(w),
         FP_SEG(w->parent), FP_OFF(w->parent),
         FP_SEG(w->children), FP_OFF(w->children),
         FP_SEG(w->siblings), FP_OFF(w->siblings) );
      printk(buf);
      if(w->children)
         XIDumpAncestryChain(w->children, indent+1);
   }
   if(!indent)
      printk("<-XIDumpAncestryChain\r\n");
   unlockTask(flag);
}

void far XIDumpDisplayChain(void)
{
wininfo_t far *w = topWin;
char buf[80];
short flag;

   flag = lockTask();
   printk("->XIDumpDisplayChain\r\n");
   while(w)
   {
      sprintf(buf, "w = %x:%x, w->up = %x:%x, w->down = %x:%x\r\n",
         FP_SEG(w), FP_OFF(w),
         FP_SEG(w->up), FP_OFF(w->up),
         FP_SEG(w->down), FP_OFF(w->down) );
      printk(buf);
      w = w->down;
   }
   printk("<-XIDumpDisplayChain\r\n");
   unlockTask(flag);
}


/*------------------------------------------------------------------------
   Public library functions
------------------------------------------------------------------------*/

Window far XCreateSimpleWindow(Display far *dpy, Window parent,
                           int x, int y, int w, int h,
                           long bw, long bd, long bg)
{
wininfo_t far *newWin;
short flag;

/* Sanity check */
   if(!dpy || !parent)
      return (Window)NULL;

/* allocate a new window structure */
   if((newWin = (wininfo_t far *)malloc(sizeof(wininfo_t))) == NULL)
   {
      printk("XCreateSimpleWindow: out of memory\r\n");
      return (Window)NULL;
   }

/* fill out wininfo_t structure from args */
   newWin->x = x;
   newWin->y = y;
   newWin->w = w;
   newWin->h = h;
   newWin->bw = (int)bw;
   newWin->bd = bd;
   newWin->bg = bg;
   newWin->event_mask = 0;

/* link into ownership chain */
   flag = lockTask();
   newWin->owner = dpy;
   newWin->next = (wininfo_t far *)dpy->resources.windows;
   newWin->prev = NULL;
   if(newWin->next)
      newWin->next->prev = newWin;
   dpy->resources.windows = (Window)newWin;

/* link into ancestry chain */
   newWin->parent = (wininfo_t far *)parent;
   newWin->children = NULL;
   newWin->siblings = newWin->parent->children;
   newWin->parent->children = newWin;

/* clear display hierarchy pointers */
   newWin->up = newWin->down = NULL;
   unlockTask(flag);
   return (Window)newWin;
}


int far XDestroyWindow(Display far *dpy, Window win)
{
wininfo_t far *p, far *w = (wininfo_t far *)win;
short flag;

/* Sanity check */
   if(!dpy || !win || !w->parent)
      return False;

/* First unmap the window */
   if(w->down)
      XUnmapWindow(dpy, win);

/* Unhook ownership chain */
   flag = lockTask();
   if(w->prev)
      w->prev->next = w->next;
   else
      ((wininfo_t far *)dpy->resources.windows) = w->next;
   if(w->next)
      w->next->prev = w->prev;

/* Unhook ancestry chain */
   p = w->parent;

/* Part 1: remove this window from children->siblings chain of parent */
   if(p->children == w)
      p->children = w->siblings;
   else
   {
      for(p = p->children; p->siblings && p->siblings != w; p = p->siblings)
         ;
      if(!p->siblings)
      {
         printk("XDestroyWindow: Invalid sibling chain!\r\n");
         unlockTask(flag);
         return False;
      }
      p->siblings = w->siblings;
   }

/* Part 2: Attach all children of this window to root window */
   for(p = w->children; p; p = p->siblings)
   {
      p->parent = &X11rootWin;
      if(!p->siblings)
      {
         p->siblings = X11rootWin.children;
         X11rootWin.children = w->children;
         break;
      }
   }
   unlockTask(flag);

/* Free the memory */
   free(w);

/* Done! */
   return True;
}


int far XMapWindow(Display far *dpy, Window win)
{
wininfo_t far *w = (wininfo_t far *)win;
XEvent expose;
short flag;

/* Place window at top of display stack */
   if(w != topWin)
   {
      flag = lockTask();
      if(w->up)
         w->up->down = w->down;
      if(w->down)
         w->down->up = w->up;
      topWin->up = w;
      w->down = topWin;
      w->up = NULL;
      topWin = w;
      unlockTask(flag);
   }

/* Draw Window background and border using primitives */
   X11pDrv->FillRectangle(w->x+w->bw, w->y+w->bw,
               w->x+w->w, w->y+w->h, w->bg, AllPlanes);
   X11pDrv->DrawRectangle(w->x, w->y, w->x+w->w-w->bw,
               w->y+w->h-w->bw, w->bd, AllPlanes);

/* Generate an expose event, on window */
   if(w->event_mask & ExposureMask)
   {
      expose.type = Expose;
      expose.xexpose.send_event = False;
      expose.xexpose.x = w->x+w->bw;
      expose.xexpose.y = w->y+w->bw;
      expose.xexpose.width = w->w-w->bw*2;
      expose.xexpose.height = w->h-w->bw*2;
      XIQueueEvent(w, &expose);
   }
   return True;
}


int far XUnmapWindow(Display far *dpy, Window win)
{
wininfo_t far *c, far *w = (wininfo_t far *)win;
XEvent expose;
short flag;

/* Remove window from display stack */
   flag = lockTask();
   if(w->up)
      w->up->down = w->down;
   if(w->down)
      w->down->up = w->up;
   if(topWin == w)
      topWin = w->down;
   c = w->down;
   w->up = w->down = NULL;
   unlockTask(flag);

/* Generate an expose event on all uncovered windows */
   expose.type = Expose;
   expose.xexpose.x = w->x;
   expose.xexpose.y = w->y;
   expose.xexpose.width = w->w;
   expose.xexpose.height = w->h;
   XIChainDisplayEvent(c, &expose, DIR_UP);
   return True;
}

/* End */
