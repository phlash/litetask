/*------------------------------------------------------------------------
   VM19DRV.C - LiteTask X11 graphics driver for mode 19 (320x200x256)

   $Author:   Phlash  $
   $Date:   04 Jul 1994 19:19:06  $
   $Revision:   1.2  $

------------------------------------------------------------------------*/

#include "litetask.h"
#include "prims.h"
#include "font.h"
#include "colors.h"
#include "dacregs.h"

/*
Standard (IBM) font for this mode
*/
#define FONT_NAME ibm_font
#include "ibm_fnt.h"
#undef FONT_NAME

/*
Macro to return absolute value
*/
#define Abs(x) (((x) < 0) ? -(x) : (x))

/*
Exported driver interface
*/
primsDriver_t vm19drv = {
                        "(AshbySoft *) 320x200 VGA/MCGA Video Driver $Revision:   1.2  $",
                        320, 200,
                        8,
                        _InitDriver,
                        _RemoveDriver,
                        _DrawPoint,
                        _DrawLine,
                        _DrawRectangle,
                        _FillRectangle,
                        _DrawString,
                        _DrawPixmap,
                        _DrawBitmap,
                        _GetPixel, 
                        _ChangeColormap };

/*
Screen buffer pointer
*/
static unsigned char far *screen = (unsigned char far *)0xA0000000L;

/*
Local copy of hardware colormap
*/
static unsigned long cmap[256];

/*
Functions follow
*/
static void far _InitDriver(void)
{
union REGS regs;
int i;
short flag;

/* Use BIOS to change video mode */
   flag = lockTask();
   regs.x.ax = 19;
   int86(0x10, &regs);

/* Now set default colormap */
   for(i=0; i<256; i++)
   {
      if(i<_MAXBIOS_PIX)
         cmap[i] = _BIOSCOLOR(i);
      else
         cmap[i] = _BLACK;
   }
   setDACregisters(cmap, 0, 256);
   unlockTask(flag);
}

static void far _RemoveDriver(void)
{
union REGS regs;
short flag = lockTask();

/* Change back to text mode */
   regs.x.ax = 3;
   int86(0x10, &regs);
   unlockTask(flag);
}

static void near _Swap(unsigned short far *p1, unsigned short far *p2)
{
unsigned short tmp;

   tmp = *p1;
   *p1 = *p2;
   *p2 = tmp;
}

/*
 * Draw a single point (pixel)
 */
static void far _DrawPoint(unsigned short x, unsigned short y,
               unsigned long pix, unsigned long pmask)
{
register unsigned char p;
/* read current pixel value */
   p = screen[x+320*y] & ~((unsigned char)pmask);

/* set pixel with plane mask */
   screen[x+320*y] = p | (unsigned char)(pix & pmask);
}

/*
 * Draw a horizontal line
 */
static void near _DrawHorizLine(unsigned short x1, unsigned short x2,
               unsigned short y, unsigned char pix, unsigned char pmask)
{
register unsigned short x, yoff;

/* make sure that x1 <= x2 */
   if(x1 > x2)
      _Swap(&x1, &x2);

   yoff = y*320;
   for(x=x1; x<=x2; x++)
      screen[x+yoff] = (screen[x+yoff] & ~pmask) | (pix & pmask);
}

/*
 * Draw a vertical line
 */
static void near _DrawVertLine(unsigned short x, unsigned short y1,
               unsigned short y2, unsigned char pix, unsigned char pmask)
{
register unsigned short yoff, yend;

/* make sure that y1 <= y2 */
   if(y1 > y2)
      _Swap(&y1, &y2);

   yend = y2*320;
   for(yoff=y1*320; yoff<=yend; yoff += 320)
      screen[x+yoff] = (screen[x+yoff] & ~pmask) | (pix & pmask);
}

/*
 * Draw a sloped line (Bresenhams algorithm) from X-axis baseline
 */
static void near _DrawSlopeLineX(unsigned short x1, unsigned short y1,
               unsigned short x2, unsigned short y2, unsigned char pix,
               unsigned char pmask)
{
register unsigned short x, yoff;
short d, dx, dy, Aincr, Bincr, yincr;

/* Bresenhams line algorithm for slopes between -1 to +1 relative to X */

/* make sure that x1 <= x2 */
   if(x1 > x2)
   {
      _Swap(&x1, &x2);
      _Swap(&y1, &y2);
   }
   
/* calculate constants */
   dx = x2 - x1;
   if(y2 > y1)
   {
      yincr = 1;
      dy = y2 - y1;
   }
   else
   {
      yincr = -1;
      dy = y1 - y2;
   }

   d  = 2 * dy - dx;

   Aincr = 2 * (dy - dx);
   Bincr = 2 * dy;

   x = x1;
   yoff = y1*320;
   
   screen[x+yoff] = (screen[x+yoff] & ~pmask) | (pix & pmask);

   for(x=x1+1; x<=x2; x++)
   {
      if(d >= 0)
      {
         yoff += yincr * 320;
         d += Aincr;
      }
      else
         d += Bincr;

      screen[x+yoff] = (screen[x+yoff] & ~pmask) | (pix & pmask);
   }
}

/*
 * Draw a sloped line (Bresenhams algorithm) from Y-axis baseline
 */
static void near _DrawSlopeLineY(unsigned short x1, unsigned short y1,
               unsigned short x2, unsigned short y2, unsigned char pix,
               unsigned char pmask)
{
register unsigned short x, yoff, yend;
short d, dx, dy, Aincr, Bincr, xincr;

/* Bresenhams line algorithm for slopes between -1 to +1 relative to Y */

/* make sure that y1 <= y2 */
   if(y1 > y2)
   {
      _Swap(&x1, &x2);
      _Swap(&y1, &y2);
   }
   
/* calculate constants */
   dy = y2 - y1;
   if(x2 > x1)
   {
      xincr = 1;
      dx = x2 - x1;
   }
   else
   {
      xincr = -1;
      dx = x1 - x2;
   }

   d  = 2 * dx - dy;

   Aincr = 2 * (dx - dy);
   Bincr = 2 * dx;

   yoff = y1*320;
   x = x1;
   
   screen[x+yoff] = (screen[x+yoff] & ~pmask) | (pix & pmask);

   yend = y2*320;
   for(yoff=(y1+1)*320; yoff<=yend; yoff+=320)
   {
      if(d >= 0)
      {
         x += xincr;
         d += Aincr;
      }
      else
         d += Bincr;

      screen[x+yoff] = (screen[x+yoff] & ~pmask) | (pix & pmask);
   }
}

/*
 * General purpose entry point for line drawing
 */
static void far _DrawLine(unsigned short x1, unsigned short y1,
               unsigned short x2, unsigned short y2,
               unsigned long pix, unsigned long pmask)
{
short dx, dy;

/* check for horizontal or vertical lines */
   if(x1 == x2)
   {
      _DrawVertLine(x1, y1, y2, (unsigned char)pix, (unsigned char)pmask);
      return;
   }
   if(y1 == y2)
   {
      _DrawHorizLine(x1, x2, y1, (unsigned char)pix, (unsigned char)pmask);
      return;
   }

/* check slope of line */
   dx = (signed)x2 - (signed)x1;
   dy = (signed)y2 - (signed)y1;
   dx = Abs(dx);
   dy = Abs(dy);
   if(dy > dx)               // Slope > 1:1 draw relative to Y axis
   {
      _DrawSlopeLineY(x1, y1, x2, y2, (unsigned char)pix, (unsigned char)pmask);
      return;
   }
   else                      // Slope <= 1:1 draw relative to X axis
   {
      _DrawSlopeLineX(x1, y1, x2, y2, (unsigned char)pix, (unsigned char)pmask);
      return;
   }
}

/*
 * Draw a rectangle (from any opposing corners)
 */
static void far _DrawRectangle(unsigned short x1, unsigned short y1,
               unsigned short x2, unsigned short y2,
               unsigned long pix, unsigned long pmask)
{
/* render by drawing four lines! */
   _DrawHorizLine(x1, x2, y1, pix, pmask);
   _DrawHorizLine(x1, x2, y2, pix, pmask);
   _DrawVertLine(x1, y1, y2, pix, pmask);
   _DrawVertLine(x2, y1, y2, pix, pmask);
}

/*
 * Fill a rectangle (from any opposing corners)
 */
static void far _FillRectangle(unsigned short x1, unsigned short y1,
               unsigned short x2, unsigned short y2,
               unsigned long pix, unsigned long pmask)
{
register unsigned char p;
register short x, yoff, y;

/* ensure that (y1 <= y2) & (x1 <= x2) */
   if(y1 > y2)
      _Swap(&y1, &y2);
   if(x1 > x2)
      _Swap(&x1, &x2);

/* render by drawing lots of horizontal lines (directly, not DrawLine) */
   for(y=y1; y<=y2; y++)
   {
      yoff = y*320;
      for(x=x1; x<=x2; x++)
      {
         p = screen[x+yoff] & ~((unsigned char)pmask);
         screen[x+yoff] = p | ((unsigned char)pix & (unsigned char)pmask);
      }
   }
}

/*
 * Draw a text string
 */
static void far _DrawString(unsigned short x, unsigned short y, char far *str, int len,
               unsigned long pix, unsigned long pmask)
{
int i, lx, ly;
unsigned char c;

   for(i=0; i<len; i++)
   {
   // Render character on screen
      c = str[i];
      for(ly = 0; ly < 8; ly++)
         for(lx = 0; lx < 8; lx++)
            if(ibm_font[c][ly] & (1 << lx))
               _DrawPoint(x + lx, y + ly, pix, pmask);

   // Increment x, check for wrap
      x += 8;
      if(x > 319-8)
      {
         x = 0;
         y = (y + 8) % 192;
      }
   }
}

/*
 * Draw a Pixmap (multi-color pixel array)
 */
static void far _DrawPixmap(unsigned short x, unsigned short y,
                                unsigned short w, unsigned short h,
                                unsigned long far *pixmap, unsigned long pmask)
{
register unsigned char p;
unsigned short yoff, lx, ly;

/* Copy pixel values to the screen */
   for(ly=0; ly<h; ly++)
   {
      yoff = (ly+y)*320+x;
      for(lx=0; lx<w; lx++)
      {
         p = screen[lx+yoff] & ~((unsigned char)pmask);
         screen[lx+yoff] = p | ((unsigned char)pixmap[ly*w+lx] & (unsigned char)pmask);
      }
   }
}

/*
 * Draw a Bitmap (single color, one bit per pixel)
 */
static void far _DrawBitmap(unsigned short x, unsigned short y,
                                unsigned short w, unsigned short h,
                                unsigned char far *bitmap,
                                unsigned long pix, unsigned long pmask)
{
register unsigned char p, bitmask;
unsigned short yoff, byteWidth, bitmapIndex, lx, ly;

/* Configure byteWidth */
   byteWidth = (w+7)/8;

/* Copy pixel values to the screen */
   for(ly=0; ly<h; ly++)
   {
      yoff = (ly+y)*320+x;
      bitmapIndex = byteWidth*ly;
      bitmask = 0x80;
      for(lx=0; lx<w; lx++)
      {
         if(bitmap[bitmapIndex] & bitmask)
         {
            p = screen[lx+yoff] & ~((unsigned char)pmask);
            screen[lx+yoff] = p | ((unsigned char)pix & (unsigned char)pmask);
         }
         if(bitmask == 1)
         {
            bitmask = 0x80;
            bitmapIndex++;
         }
         else
            bitmask >>= 1;
      }
   }
}

/*
 * Read back a pixel value from the screen
 */
static unsigned long far _GetPixel(unsigned short x, unsigned short y, unsigned long pmask)
{
/* return masked pixel value at location specified */
   return (unsigned long)(screen[x+y*320]) & pmask;
}

/*
 * Change the hardware colormap
 */
static void far _ChangeColormap(unsigned short ncols, cmap_t far *cols)
{
unsigned short i, lp, hp;
short flag;

/* update local copy */
   flag = lockTask();
   lp = 255;
   hp = 0;
   for(i=0; i<ncols; i++)
   {
      lp = min(lp, (short)cols[i].pixel);
      hp = max(hp, (short)cols[i].pixel);
      cmap[(short)cols[i].pixel] = cols[i].rgb;
   }

/* now update hardware */
   setDACregisters(&cmap[lp], lp, hp-lp+1);
   unlockTask(flag);
}

/* End */
