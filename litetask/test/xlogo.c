#include "litetask.h"
#include "Xlib.h"
#include "../X11/Xlib/Xint.h"

int mainStackSize = MINSTACK;

char msgBuf[8192];

void dumpMsgBuf(char far *name)
{
int fd;

   fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
   if(fd < 0)
      return;
   write(fd, msgBuf, strlen(msgBuf));
   close(fd);
}

void far mainTask(void)
{
Display far *dpy;
Window root, win1, win2, win3;
GC gc;
short x, y;
int fsId;

/* set up kernel message buffer */
   memset(msgBuf, 0, sizeof(msgBuf));
   setPrintk(NULL, msgBuf, sizeof(msgBuf));

/* Create file system for drive c: */
   initFileSystem();
   initFatFs();
   fsId=newFatFs(2, 0L, "c:");
   if(fsId < 0)
      return;

/* Start X Windows system (320x200) */
   InstallXWindows(&vm19drv);
   dpy = XOpenDisplay("VGA");
   root = DefaultRootWindow(dpy);
   gc = DefaultGC(dpy, 0);

   XIDumpOwnershipChain(dpy);
   XIDumpAncestryChain((wininfo_t far *)root, 0);
   XIDumpDisplayChain();
   dumpMsgBuf("c:xlogo1.msg");

   delayTask(18L);
   win1 = XCreateSimpleWindow(dpy, root, DisplayWidth(dpy, 0)/2-50,
            DisplayHeight(dpy, 0)/2-50, 100, 100, 1, WhitePixel(dpy, 0),
            BlackPixel(dpy, 0));
   XMapWindow(dpy, win1);
   XSetForeground(dpy, gc, WhitePixel(dpy, 0));
   for(y=0; y<100; y+=5)
      XDrawLine(dpy, win1, gc, 0, y, 99, y);
   for(x=0; x<100; x+=5)
      XDrawLine(dpy, win1, gc, x, 0, x, 99);

   win2 = XCreateSimpleWindow(dpy, root, 50,
            50, 100, 100, 1, WhitePixel(dpy, 0),
            BlackPixel(dpy, 0));
   XMapWindow(dpy, win2);

   win3 = XCreateSimpleWindow(dpy, win2, 150, 50, 100, 100, 1,
            WhitePixel(dpy, 0), BlackPixel(dpy, 0));
   XMapWindow(dpy, win3);

   XIDumpOwnershipChain(dpy);
   XIDumpAncestryChain((wininfo_t far *)root, 0);
   XIDumpDisplayChain();
   dumpMsgBuf("c:xlogo2.msg");

   delayTask(18L);
   XDestroyWindow(dpy, win2);

   XIDumpOwnershipChain(dpy);
   XIDumpAncestryChain((wininfo_t far *)root, 0);
   XIDumpDisplayChain();
   dumpMsgBuf("c:xlogo3.msg");

   delayTask(18L);
   XCloseDisplay(dpy);

   XIDumpAncestryChain((wininfo_t far *)root, 0);
   XIDumpDisplayChain();
   dumpMsgBuf("c:xlogo4.msg");

   delayTask(18L);
   RemoveXWindows();
   
/* Remove file systems support */
   deleteFatFs(fsId);
   removeFatFs();
   removeFileSystem();
}

