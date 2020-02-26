/*------------------------------------------------------------------------
   XINT.H - LiteTask X11 Library internal interface definitions

   $Author:$
   $Date:$
   $Revision:$

------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
   Notes:

   1/ Assumes 14" monitor, I need a way of configuring this information.
   2/ Events queued in Display structure, since XNextEvent(Display far *,);

------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
Constants for Xlib/display
------------------------------------------------------------------------*/
#define _WIDTH_MM  260          // Assumes 14" VGA monitor (!)
#define _HEIGHT_MM 195          // Assumes 14" VGA monitor (!)

#define _MAX_COLORS 256         // Maximum size of a Colormap
#define _READ_ONLY 0            // Colormap entry flags
#define _AVAILABLE 1
#define _ALLOCATED 2

#define DIR_UP     0            // Hierarchy search directions
#define DIR_DOWN   1

/*------------------------------------------------------------------------
   Macros
------------------------------------------------------------------------*/
#define Abs(x) (((x) < 0) ? -(x) : (x))
#define PointInWindow(px, py, win)  ((px) >= (win)->x &&           \
                                     (px) < ((win)->x + (win)->w) && \
                                     (py) >= (win)->y &&           \
                                     (py) < ((win)->y + (win)->h) )

/*------------------------------------------------------------------------
   Internal types
------------------------------------------------------------------------*/
typedef struct rectangle_tag {
         int x,y,w,h;
         } rectangle_t;


/*------------------------------------------------------------------------
   wininfo_t structure notes:

      Window ownership maintained by pointer to Display structure, and chain
      of owned windows via 'next' & 'prev' pointers.

      Window ancestry hierarchy maintained by parent/child/sibling pointer
      chains like so (three windows + root):

      Root.children --> Sub1.children --> Leaf1.children --> NULL
                    <-- Sub1.parent   <-- Leaf1.parent
                        Sub1.siblings     Leaf1.siblings
                               |                  |
                               v                  v
                        Sub2.children --> NULL   NULL
                    <-- Sub2.parent
                        Sub2.siblings
                               |
                               v
                             NULL

      Window display hierarchy maintained by up/down pointer chain like so
      (three windows + root):

            Root.down --> Sub1.down --> Leaf1.down --> Sub2.down --> NULL
      NULL<-Root.up   <-- Sub1.up   <-- Leaf1.up   <-- Sub2.up
------------------------------------------------------------------------*/

typedef struct wininfo_tag {
         int x,y,w,h;
         int bw;
         unsigned long bd, bg;
         int event_mask;
         
         /* Ownership pointers */
         Display far *owner;
         struct wininfo_tag far *next, far *prev;

         /* Ancestry hierarchy pointers */
         struct wininfo_tag far *parent, far *children, far *siblings;

         /* Display hierarchy pointers */
         struct wininfo_tag far *down, far *up;
         } wininfo_t;

typedef struct gc_tag {
         unsigned long fg;
         unsigned long bg;
         unsigned long pmask;
         struct gc_tag far *next;
         } gc_t;

typedef struct colormap_tag {
         unsigned short size;
         unsigned short installed;
         cmap_t far *map;
         char far *flags;
         struct colormap_tag far *next;
         } colormap_t;

/*------------------------------------------------------------------------
   Internal library globals
------------------------------------------------------------------------*/
extern char far *X11name;
extern primsDriver_t far *X11pDrv;
extern wininfo_t X11rootWin;

/*------------------------------------------------------------------------
   Internal library functions
------------------------------------------------------------------------*/
int far XIRectanglesOverlap(rectangle_t far *r1, rectangle_t far *r2);

int far XIQueueEvent(wininfo_t far *window, XEvent far *event);
int far XIChainDisplayEvent(wininfo_t far *start, XEvent far *event, int dir);
void far XIDumpOwnershipChain(Display far *dpy);
void far XIDumpAncestryChain(wininfo_t far *start, int indent);
void far XIDumpDisplayChain(void);

int far XIInstallColormap(void);
void far XIRemoveColormap(void);
Colormap far XIDefaultColormap(void);

GC far XIDefaultGC(void);

int far XIRootEvent(XEvent far *event);

/* End */
