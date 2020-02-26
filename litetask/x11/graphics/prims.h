/*------------------------------------------------------------------------
   PRIMS.H - LiteTask X11 graphics driver primitives

   $Author:   Phlash  $
   $Date:   04 Jul 1994 19:18:34  $
   $Revision:   1.3  $

------------------------------------------------------------------------*/

/* Function prototypes (must be static to avoid a clash) */
static void far _InitDriver(void);
static void far _RemoveDriver(void);
static void far _DrawPoint(unsigned short x, unsigned short y,
               unsigned long pix, unsigned long pmask);
static void far _DrawLine(unsigned short x1, unsigned short y1,
               unsigned short x2, unsigned short y2,
               unsigned long pix, unsigned long pmask);
static void far _DrawRectangle(unsigned short x1, unsigned short y1,
               unsigned short x2, unsigned short y2,
               unsigned long pix, unsigned long pmask);
static void far _FillRectangle(unsigned short x1, unsigned short y1,
               unsigned short x2, unsigned short y2,
               unsigned long pix, unsigned long pmask);
static void far _DrawString(unsigned short x, unsigned short y, char far *str,
               int len, unsigned long pix, unsigned long pmask);
static void far _DrawPixmap(unsigned short x, unsigned short y,
                                unsigned short w, unsigned short h,
                                unsigned long far *pixmap, unsigned long pmask);
static void far _DrawBitmap(unsigned short x, unsigned short y,
                                unsigned short w, unsigned short h,
                                unsigned char far *bitmap,
                                unsigned long pix, unsigned long pmask);
static unsigned long far _GetPixel(unsigned short x, unsigned short y,
               unsigned long pmask);
static void far _ChangeColormap(unsigned short ncols, cmap_t far *cols);

/* End */
