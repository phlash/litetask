/*------------------------------------------------------------------------
   XLIB.H - LiteTask XWindows compatible graphics interface header.

   $Author:   Phlash  $
   $Date:   05 Mar 1995 20:51:20  $
   $Revision:   1.2  $

------------------------------------------------------------------------*/

/* Constants */
#define True  1
#define False 0
#define AllPlanes 255L
#define Convex 0
#define Nonconvex 1
#define Complex 2
#define CoordModeOrigin 0

/* Client handles */
typedef void far *Pixmap;
typedef void far *Window;
typedef void far *Colormap;
typedef void far *Drawable;
typedef void far *GC;

/* Events */
#define ExposureMask 1L
#define Expose 1

typedef struct {
         int type;
         int send_event;
         Window window;
         int x, y;
         int width, height;
         } XAnyEvent;

typedef struct {
         int type;
         int send_event;
         Window window;
         int x, y;
         int width, height;
         } XExposeEvent;

typedef union {
         int type;
         XAnyEvent xany;
         XExposeEvent xexpose;
         char pad[32];
         } XEvent;

/* Data structures */
typedef struct {
         unsigned short x, y;
         } XPoint;

typedef struct {
         unsigned long pixel;
         unsigned long rgb;
         } XColor;

/* The Display structure */
typedef struct {
            Window windows;
            GC gcs;
            Colormap cmaps;
            } resource_t;

#define XQUEUE_SIZE  4
typedef struct {
            char far *name;
            unsigned short width, height, wMM, hMM, planes;
            unsigned long black_pixel, white_pixel;
            Colormap colormap;
            GC gc;
            Window root_window;
            resource_t resources;
            taskHandle pending;
            unsigned short head, tail;
            XEvent eventQueue[XQUEUE_SIZE];
            } Display;

/* Macros */
#define DisplayWidth(dpy,scr)           (((Display far *)dpy)->width)
#define DisplayWidthMM(dpy,scr)         (((Display far *)dpy)->wMM)
#define DisplayHeight(dpy,scr)          (((Display far *)dpy)->height)
#define DisplayHeightMM(dpy,scr)        (((Display far *)dpy)->hMM)
#define DisplayPlanes(dpy,scr)          (((Display far *)dpy)->planes)
#define RootWindow(dpy,scr)             (((Display far *)dpy)->root_window)
#define DefaultRootWindow(dpy)          RootWindow(dpy,0)
#define DefaultColormap(dpy,scr)        (((Display far *)dpy)->colormap)
#define DefaultScreen(dpy) 0
#define DefaultGC(dpy, scr)             (((Display far *)dpy)->gc)
#define BlackPixel(dpy,scr)             (((Display far *)dpy)->black_pixel)
#define WhitePixel(dpy,scr)             (((Display far *)dpy)->white_pixel)
#define XDisplayName(dpy)               (((Display far *)dpy)->name)

/* Additions to standard Xlib, these install the X Windows 'server' */
int far InstallXWindows(void far *driver);
int far RemoveXWindows(void);

/* Xlib compatible graphics functions */
Display far * far XOpenDisplay(char far *);
void far XCloseDisplay(Display far *);

Window far XCreateSimpleWindow(Display far *, Window, int, int, int, int, long, long, long);
int far XDestroyWindow(Display far *, Window);

GC far XCreateGC(Display far *, Drawable, unsigned long, void far *);
int far XFreeGC(Display far *, GC);

Pixmap far XCreatePixmap(Display far *, Drawable, int, int, int);
int far XFreePixmap(Display far *, Pixmap);

/* all return True on success False on error */
int far XMapWindow(Display far *, Window);
int far XUnmapWindow(Display far *, Window);

int far XSelectInput(Display far *, Window, long);
int far XNextEvent(Display far *, XEvent far *);
int far XSendEvent(Display far *, Window, int, long, XEvent far *);

int far XParseColor(Display far *, Colormap, char far *, XColor far *);
int far XAllocColor(Display far *, Colormap, XColor far *);
int far XAllocColorCells(Display far *, Colormap, int, unsigned long far *, int, unsigned long far *, int);
int far XFreeColors(Display far *, Colormap, unsigned long far *, int, unsigned long);

int far XStoreColor(Display far *, Colormap, XColor);
int far XStoreColors(Display far *, Colormap, XColor far *, int);

int far XDrawPoint(Display far *, Drawable, GC, int, int);
int far XSetForeground(Display far *, GC, unsigned long);
int far XDrawRectangle(Display far *, Drawable, GC, int, int, int, int);
int far XFillRectangle(Display far *, Drawable, GC, int, int, int, int);
int far XDrawLine(Display far *, Drawable, GC, int, int, int, int);
int far XDrawLines(Display far *, Drawable, GC, XPoint far *, int, int);
int far XCopyArea(Display far *, Drawable, Drawable, GC, int, int, int, int, int, int);
int far XFillPolygon(Display far *, Drawable, GC, XPoint far *, int, int, int);
int far XDrawString(Display far *, Drawable, GC, int, int, char far *, int);
int far XSetPlaneMask(Display far *, GC, unsigned long);

/* no return values */
void far XSync(Display far *, int);
void far XFlush(Display far *);

/* End */

