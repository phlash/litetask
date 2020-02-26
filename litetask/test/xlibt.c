/* Test program for ASHBYSOFT X/VGA Xlib emulator */
#include "litetask.h"
#include "Xlib.h"

//#define USE_COLOUR_PLANES
#define DELAY 3L
#define LOOPS (90/(int)DELAY)

int mainStackSize = MINSTACK;

Display far *dpy;
short width, height;

void DrawObject1(Display far *dpy, Window win, GC gc, unsigned long fg,
   unsigned long bg, short x, short y)
{
/* Clear rectangle */
   XSetForeground(dpy, gc, bg);
   XFillRectangle(dpy, win, gc, x, y, width/5, height/5);

/* Draw two filled boxes */
   XSetForeground(dpy, gc, fg);
   XFillRectangle(dpy, win, gc, x, y, width/10, height/10);
   XFillRectangle(dpy, win, gc, x+width/10, y+height/10, width/10, height/10);

/* Done */
}

void DrawObject2(Display far *dpy, Window win, GC gc, unsigned long fg,
   unsigned long bg, short x, short y)
{
/* Clear rectangle */
   XSetForeground(dpy, gc, bg);
   XFillRectangle(dpy, win, gc, x, y, width/5, height/5);

/* Draw a crossed box */
   XSetForeground(dpy, gc, fg);
   XDrawRectangle(dpy, win, gc, x, y, width/5, height/5);
   XDrawLine(dpy, win, gc, x, y, width/5, height/5);

/* Done */
}

void far renderTask(int x, int y)
{
Window win;
GC gc1, gc2;
XColor colorSets[2][4];
XColor red, blue, white, gray;
unsigned long pixels[1], planes[2];
char buf[80];
int i;

/* Get some rgb values */
   XParseColor(dpy, DefaultColormap(dpy,0), "blue", &blue);
   XParseColor(dpy, DefaultColormap(dpy,0), "red", &red);
   XParseColor(dpy, DefaultColormap(dpy,0), "white", &white);
   XParseColor(dpy, DefaultColormap(dpy,0), "gray", &gray);

#ifdef USE_COLOUR_PLANES
/* Create two GCs */
   gc1 = XCreateGC(dpy, DefaultRootWindow(dpy), 0L, NULL);
   if(!gc1)
   {
      printk("Error, cannot create GC\r\n");
      return;
   }
   gc2 = XCreateGC(dpy, DefaultRootWindow(dpy), 0L, NULL);
   if(!gc2)
   {
      printk("Error, cannot create GC\r\n");
      return;
   }
   
/* Allocate 1 colour in two overlay planes (4 colour cells) */
   if(XAllocColorCells(dpy, DefaultColormap(dpy, 0), False,
      planes, 2, pixels, 1) == 0)
   {
      printk("Unable to allocate color cells\r\n");
      return;
   }

/* display info returned */
   sprintf(buf, "X:%i Y:%i, pixels[0]=%l, planes[0]=%l, planes[1]=%l\r\n",
      x, y, pixels[0], planes[0], planes[1]);
   printk(buf);

/* fill out allocated color cells */
   colorSets[0][0].pixel = pixels[0];
   colorSets[0][1].pixel = pixels[0] | planes[0];
   colorSets[0][2].pixel = pixels[0] | planes[1];
   colorSets[0][3].pixel = pixels[0] | planes[0] | planes[1];

   colorSets[0][0].rgb = blue.rgb;
   colorSets[0][1].rgb = red.rgb;
   colorSets[0][2].rgb = blue.rgb;
   colorSets[0][3].rgb = red.rgb;

   colorSets[1][0].pixel = pixels[0];
   colorSets[1][1].pixel = pixels[0] | planes[0];
   colorSets[1][2].pixel = pixels[0] | planes[1];
   colorSets[1][3].pixel = pixels[0] | planes[0] | planes[1];

   colorSets[1][0].rgb = gray.rgb;
   colorSets[1][1].rgb = gray.rgb;
   colorSets[1][2].rgb = white.rgb;
   colorSets[1][3].rgb = white.rgb;

/* Fill in GC plane masks */
   XSetPlaneMask(dpy, gc1, pixels[0] | planes[0]);
   XSetPlaneMask(dpy, gc2, pixels[0] | planes[1]);

/* Set first color set into map (view plane 1) */
   XStoreColors(dpy, DefaultColormap(dpy, 0), colorSets[1], 4);

#else
/* Allocate 1 local GC (we don't want to change the default) */
   gc1 = XCreateGC(dpy, DefaultRootWindow(dpy), 0L, NULL);
   if(!gc1)
   {
      printk("Error, unable to create GC\r\n");
      return;
   }
   XSetPlaneMask(dpy, gc1, AllPlanes);

/* Allocate four read-only colors to draw with */
   XAllocColor(dpy, DefaultColormap(dpy, 0), &red);
   XAllocColor(dpy, DefaultColormap(dpy, 0), &blue);
   XAllocColor(dpy, DefaultColormap(dpy, 0), &white);
   XAllocColor(dpy, DefaultColormap(dpy, 0), &gray);

#endif

/* Create a window */
   win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy),
                              x, y, width/3-5, height/3-5,
                              1L, WhitePixel(dpy,0), BlackPixel(dpy,0));
   if(!win)
   {
      printk("Error, cannot create window\r\n");
      return;
   }

/* Now display Window */
   XMapWindow(dpy, win);

#ifdef USE_COLOUR_PLANES
/* Draw objects in each plane */
   DrawObject1(dpy, win, gc1, pixels[0] | planes[0], pixels[0], 10, 10);
   DrawObject2(dpy, win, gc2, pixels[0] | planes[1], pixels[0], 10, 10);

/* Now cycle colors */
   for(i=0; i<LOOPS; i++)
   {
      delayTask(DELAY);
      XStoreColors(dpy, DefaultColormap(dpy, 0), colorSets[0], 4);
      delayTask(DELAY);
      XStoreColors(dpy, DefaultColormap(dpy, 0), colorSets[1], 4);
   }
#else
/* Draw each object alternately */
   for(i=0; i<LOOPS; i++)
   {
      delayTask(DELAY);
      DrawObject1(dpy, win, gc1, red.pixel, blue.pixel, 10, 10);
      delayTask(DELAY);
      DrawObject2(dpy, win, gc1, white.pixel, gray.pixel, 10, 10);
   }
#endif
   XUnmapWindow(dpy, win);
}

void far idleHook(void)
{
char buf[40];
static int cnt = 0;
   cnt = (cnt+1) % 500;
   if(cnt)
      return;
   sprintf(buf, "Tasks: %i Idles: %i\r", getTaskSwitches(), getIdleSwitches());
   printk(buf);
}

void far runTest(void far *drv)
{
int x, y;

/* Start XWindows 'server' */
   InstallXWindows(drv);

/* Open display */
   dpy = XOpenDisplay("VGA");
   if(!dpy)
   {
      printk("Error, cannot open X display\r\n");
      return;
   }

/* initialise global variables */
   width = DisplayWidth(dpy,0)-20;
   height = DisplayHeight(dpy,0)-20;

/* Start rendering tasks */
   for(y=0; y<3; y++)
      for(x=0; x<3; x++)
         newTask(MINSTACK, renderTask, 2*sizeof(int), x * width/3, y * height/3);

/* Go to sleep until all sub-tasks die */
   do
   {
      delayTask(18L);
   }
   while(getTaskCount() > 2);

/* bye! */
   delayTask(18L);
   XCloseDisplay(dpy);
   RemoveXWindows();
}

void far mainTask()
{
int fsId;

/* usual LiteTask stuff */
   setPreEmptive(1);
   setIdleHook(idleHook);

/* Create file system for bitmaps! */
   initFileSystem();
   initFatFs();
   fsId=newFatFs(2, 0L, "c:");

/* Run the test on each driver */
   runTest(&vm18drv);
   runTest(&vm19drv);

   deleteFatFs(fsId);
   removeFatFs();
   removeFileSystem();
}

/* End */
