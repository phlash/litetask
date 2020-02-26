/*------------------------------------------------------------------------
   XEVENTS.C - LiteTask X11 Library, event handling routines

   $Author:$
   $Date:$
   $Revision:$

------------------------------------------------------------------------*/

#include "litetask.h"
#include "Xlib.h"
#include "Xint.h"

/*
 * Internal (Xlib) functions
 */
int far XIQueueEvent(wininfo_t far *window, XEvent far *event)
{
short flag;

/* Sanity check */
   if(!window || !event)
      return -1;

/* no owner => use default handler syncronously */
   if(!window->owner)
      return XIRootEvent(event);

/* lock task for critical section */
   flag = lockTask();

/* place event on owners queue, possibly wake up thread */
   event->xany.window = (Window)window;
   window->owner->eventQueue[window->owner->head++] = *event;
   if(window->owner->head >= XQUEUE_SIZE)
      window->owner->head = 0;
   if(window->owner->pending)
   {
      resumeTask(window->owner->pending, 0);
      window->owner->pending = NULL;
   }

/* done */
   unlockTask(flag);
   return 0;
}

/*
 * External (client) functions
 */
int far XSelectInput(Display far *dpy, Window win, long events)
{
wininfo_t far *w = (wininfo_t far *)win;

/* Set window owner */
   w->owner = dpy;

/* Enable specified input events */
   w->event_mask = (int)events;
   return True;
}

int far XSendEvent(Display far *dpy, Window win, int propagate,
                   long event_mask, XEvent far *event)
{
wininfo_t far *w = (wininfo_t far *)win;

/* Set send_event flag to True, then send the event to the window */
   event->xany.send_event = True;
   return XIQueueEvent(w, event);
}

int far XNextEvent(Display far *dpy, XEvent far *event)
{
short flag;

/* lock for critical section */
   flag = lockTask();

/* see if anything is on the queue */
   if(dpy->tail == dpy->head)
   {
   /* If not, suspend this thread */
      dpy->pending = getTaskHandle();
      suspendTask();
      lockTask();
   }

/* read event off the queue */
   *event = dpy->eventQueue[dpy->tail++];
   if(dpy->tail >= XQUEUE_SIZE)
      dpy->tail = 0;
   unlockTask(flag);
   return 0;
}

/* End */
