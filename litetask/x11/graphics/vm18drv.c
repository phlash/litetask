/*-----------------------------------------------------------------------
   VM18DRV.C - LiteTask X11 graphics driver for mode 18 (640x480x16)

   $Author:   Phlash  $
   $Date:   04 Jul 1994 19:18:54  $
   $Revision:   1.3  $

------------------------------------------------------------------------*/

#include "litetask.h"
#include "prims.h"
#include "font.h"
#include "colors.h"
#include "dacregs.h"

/*
VGA register set
*/
#define GC_INDEX                 0x3CE
#define SEQ_INDEX                0x3C4
#define SET_RESET_INDEX          0
#define SET_RESET_ENABLE_INDEX   1
#define READ_MAP_INDEX           4
#define GC_MODE_INDEX            5
#define COLOR_DONT_CARE          7
#define BIT_MASK_INDEX           8
#define MAP_MASK_INDEX           2

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
primsDriver_t vm18drv = {
                        "(AshbySoft *) 640x480 VGA Video Driver $Revision:   1.3  $",
                        640, 480,
                        4,
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
static unsigned long cmap[_MAXBIOS_PIX];

/*
Functions follow
*/
static void far _InitDriver(void)
{
union REGS regs;
unsigned char far *pr;
unsigned short i;
short flag;

static unsigned char EGAPal[17] =
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0 };

/* Use BIOS to change video mode */
   flag = lockTask();
   regs.x.ax = 18;
   int86(0x10, &regs);

/* set up 16 colour pages, inital page 0 */
   regs.x.ax = 0x1013;
   regs.x.bx = 0x0100;
   int86(0x10, &regs);
   regs.x.ax = 0x1013;
   regs.x.bx = 0x0001;
   int86(0x10, &regs);

/* reset the EGA palette regs */
   pr = EGAPal;
   regs.x.ax = 0x1002;
   regs.x.dx = FP_OFF(pr);
   regs.x.es = FP_SEG(pr);
   int86(0x10, &regs);

/* set up default VideoDAC palette */
   for(i=0; i<_MAXBIOS_PIX; i++)
      cmap[i] = _BIOSCOLOR(i);
   setDACregisters(cmap, 0, _MAXBIOS_PIX);

/* done */
   unlockTask(flag);
}

static void far _RemoveDriver(void)
{
union REGS regs;
short flag;

/* Change back to text mode */
   flag = lockTask();
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
unsigned int offset;
unsigned char bitmask;
short flag;

/* lock task */
   flag = lockTask();

/* set pixel mask & drawing color */
   outpw(SEQ_INDEX, ((short)pmask << 8) | MAP_MASK_INDEX);
   outpw(GC_INDEX, 0xFF00 | SET_RESET_ENABLE_INDEX);
   outpw(GC_INDEX, ((short)pix << 8) | SET_RESET_INDEX);

/* set pixel to colour value */
   offset = y * 80 + x/8;
   bitmask = (unsigned char)(0x80 >> (x%8));
   outpw(GC_INDEX, (bitmask << 8) | BIT_MASK_INDEX);
   screen[offset] |= 0xFF;

/* unlock task */
   unlockTask(flag);
}

/*
 * Draw a horizontal line
 */
static void near _DrawHorizLine(unsigned short x1, unsigned short x2,
               unsigned short y, unsigned char pix, unsigned char pmask)
{
unsigned int offset, bitmask;
short flag;

/* lock task */
   flag = lockTask();

/* make sure that x1 <= x2 */
   if(x1 > x2)
      _Swap(&x1, &x2);

/* use pixel drawing if x2-x1 less than 8 pixels */
   if(x2-x1 < 8)
   {
      for(; x1 <= x2; x1++)
         _DrawPoint(x1, y, (unsigned long)pix, (unsigned long)pmask);
      return;
   }

/* set pixel mask & drawing color */
   outpw(SEQ_INDEX, ((short)pmask << 8) | MAP_MASK_INDEX);
   outpw(GC_INDEX, 0xFF00 | SET_RESET_ENABLE_INDEX);
   outpw(GC_INDEX, ((short)pix << 8) | SET_RESET_INDEX);

/* Draw first part of line to byte boundary */
   offset = y * 80 + x1/8;
   if(x1%8)
   {
      bitmask = 0;
      for(; x1%8; x1++)
         bitmask |= (0x80 >> (x1%8));
      outpw(GC_INDEX, (bitmask << 8) | BIT_MASK_INDEX);
      screen[offset] |= 0xFF;
      offset++;
   }

/* Draw middle of the line in byte width units */
   outpw(GC_INDEX, 0xFF00 | BIT_MASK_INDEX);
   for(; x1 < x2-8; x1+=8)
      screen[offset++] |= 0xFF;

/* Draw end of line */
   bitmask = 0;
   for(; x1 <= x2; x1++)
      bitmask |= (0x80 >> (x1%8));
   outpw(GC_INDEX, (bitmask << 8) | BIT_MASK_INDEX);
   screen[offset] |= 0xFF;

/* unlock task */
   unlockTask(flag);
}

/*
 * Draw a vertical line
 */
static void near _DrawVertLine(unsigned short x, unsigned short y1,
               unsigned short y2, unsigned char pix, unsigned char pmask)
{
unsigned int offset;
short flag;

/* lock task */
   flag = lockTask();

/* make sure y1 <= y2 */
   if(y1 > y2)
      _Swap(&y1, &y2);

/* set pixel mask & drawing color */
   outpw(SEQ_INDEX, ((short)pmask << 8) | MAP_MASK_INDEX);
   outpw(GC_INDEX, 0xFF00 | SET_RESET_ENABLE_INDEX);
   outpw(GC_INDEX, ((short)pix << 8) | SET_RESET_INDEX);

/* Set bitmask */
   outpw(GC_INDEX, (0x8000 >> (x%8)) | BIT_MASK_INDEX);

/* Draw line */
   offset = y1 * 80 + x/8;
   for(; y1 <= y2; y1++)
   {
      screen[offset] |= 0xFF;
      offset += 80;
   }

/* unlock task */
   unlockTask(flag);
}

/*
 * Draw a sloped line (Bresenhams algorithm) from X-axis baseline
 */
static void near _DrawSlopeLineX(unsigned short x1, unsigned short y1,
               unsigned short x2, unsigned short y2, unsigned char pix,
               unsigned char pmask)
{
unsigned short x, offset, bitmask;
short d, dx, dy, Aincr, Bincr, yincr;
short flag;

/* Bresenhams line algorithm for slopes between -1 to +1 relative to X */
   flag = lockTask();

/* make sure that x1 <= x2 */
   if(x1 > x2)
   {
      _Swap(&x1, &x2);
      _Swap(&y1, &y2);
   }
   
/* Set pixel mask & drawing colour */
   outpw(SEQ_INDEX, ((short)pmask << 8) | MAP_MASK_INDEX);
   outpw(GC_INDEX, 0xFF00 | SET_RESET_ENABLE_INDEX);
   outpw(GC_INDEX, ((short)pix << 8) | SET_RESET_INDEX);

/* Point at bitmask register */
   outp(GC_INDEX, BIT_MASK_INDEX);

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

   offset = y1*80 + x1/8;
   
   bitmask = (0x80 >> (x%8));
   outp(GC_INDEX+1, bitmask);
   screen[offset] |= 0xFF;

   for(x=x1+1; x<=x2; x++)
   {
      if(d >= 0)
      {
         offset += yincr * 80;
         d += Aincr;
      }
      else
         d += Bincr;

      bitmask >>= 1;
      if(!bitmask)
      {
         bitmask = 0x80;
         offset++;
      }
      outp(GC_INDEX+1, bitmask);
      screen[offset] |= 0xFF;
   }
   unlockTask(flag);
}

/*
 * Draw a sloped line (Bresenhams algorithm) from Y-axis baseline
 */
static void near _DrawSlopeLineY(unsigned short x1, unsigned short y1,
               unsigned short x2, unsigned short y2, unsigned char pix,
               unsigned char pmask)
{
unsigned short y, bitmask, offset;
short d, dx, dy, Aincr, Bincr, xincr;
short flag;

/* Bresenhams line algorithm for slopes between -1 to +1 relative to Y */
   flag = lockTask();

/* make sure that y1 <= y2 */
   if(y1 > y2)
   {
      _Swap(&x1, &x2);
      _Swap(&y1, &y2);
   }
   
/* Set pixel mask & drawing colour */
   outpw(SEQ_INDEX, ((short)pmask << 8) | MAP_MASK_INDEX);
   outpw(GC_INDEX, 0xFF00 | SET_RESET_ENABLE_INDEX);
   outpw(GC_INDEX, ((short)pix << 8) | SET_RESET_INDEX);

/* Point at bitmask register */
   outp(GC_INDEX, BIT_MASK_INDEX);

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

   offset = y1*80 + x1/8;
   bitmask = (0x80 >> (x1%8));
   outp(GC_INDEX+1, bitmask);
   screen[offset] |= 0xFF;

   for(y=y1+1; y<=y2; y++)
   {
      if(d >= 0)
      {
         if(xincr > 0)
            bitmask >>= 1;
         else
            bitmask <<= 1;
         d += Aincr;
      }
      else
         d += Bincr;

      if(!bitmask)
      {
         if(xincr > 0)
         {
            offset++;
            bitmask = 0x80;
         }
         else
         {
            offset--;
            bitmask = 0x01;
         }
      }
      offset += 80;
      outp(GC_INDEX+1, bitmask);
      screen[offset] |= 0xFF;
   }
   unlockTask(flag);
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
/* ensure that y1 <= y2 */
   if(y1 > y2)
      _Swap(&y1, &y2);

/* render by drawing horizontal lines */
   for(; y1 <= y2; y1++)
      _DrawHorizLine(x1, x2, y1, pix, pmask);
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
      if(x > 639-8)
      {
         x = 0;
         y = (y + 8) % 472;
      }
   }
}

/*
 * Draw a Pixmap (set of pixel values, multi-color)
 */
static void far _DrawPixmap(unsigned short x, unsigned short y,
                                unsigned short w, unsigned short h,
                                unsigned long far *pixmap, unsigned long pmask)
{
unsigned short offset, lx, ly;
unsigned char bitmask, originalGCMode;
short flag;

/* lock the task */
   flag = lockTask();

/* use write mode 2 (pixel value=CPU byte), read mode 1 (read 0xFF) */
   outp(GC_INDEX, GC_MODE_INDEX);
   originalGCMode = inp(GC_INDEX+1);
   outp(GC_INDEX+1, originalGCMode | 0x0A);

/* set color don't care to zero, ensures 0xFF is always read from VGA */
   outpw(GC_INDEX, 0 | COLOR_DONT_CARE);
   
/* set pixel mask in sequencer map mask */
   outpw(SEQ_INDEX, ((short)pmask << 8) | MAP_MASK_INDEX);

/* select bitmask register */
   outp(GC_INDEX, BIT_MASK_INDEX);

/* write pixmap to screen */
   for(ly=0; ly<h; ly++)
   {
      offset = (ly+y)*80 + (x/8);
      bitmask = (unsigned char)(0x80 >> (x % 8));
      for(lx=0; lx<w; lx++)
      {
         outp(GC_INDEX+1, bitmask);
         screen[offset] &= (unsigned char)pixmap[lx+w*ly];
         if( bitmask & 1 )
         {
            bitmask = 0x80;
            offset++;
         }
         else
            bitmask >>= 1;
      }
   }

/* reset read/write mode */
   outpw(GC_INDEX, (originalGCMode << 8) | GC_MODE_INDEX);
   outpw(GC_INDEX, 0x0F00 | COLOR_DONT_CARE);

/* unlock the task */
   unlockTask(flag);
}

/*
 * Draw a Bitmap (single color, one bit-per-pixel)
 */
static void far _DrawBitmap(unsigned short x, unsigned short y,
                                unsigned short w, unsigned short h,
                                unsigned char far *bitmap,
                                unsigned long pix, unsigned long pmask)
{
unsigned short offset, lx, ly, byteWidth;
unsigned short lShift, rShift, bitmapIndex;
register unsigned char mask, screenByte;
unsigned char originalGCMode;
short flag;

/* lock task */
   flag = lockTask();

/* use write mode 3 (bitmask value=CPU byte), read mode 1 (read 0xFF) */
   outp(GC_INDEX, GC_MODE_INDEX);
   originalGCMode = inp(GC_INDEX+1);
   outp(GC_INDEX+1, originalGCMode | 0x0B);

/* set color don't care to zero, ensures 0xFF is always read from VGA */
   outpw(GC_INDEX, 0 | COLOR_DONT_CARE);
   
/* set bitmask register to 0xFF, ensures CPU data is not masked */
   outpw(GC_INDEX, 0xFF00 | BIT_MASK_INDEX);

/* set drawing colour & pixel mask */
   outpw(GC_INDEX, 0xFF00 | SET_RESET_ENABLE_INDEX);
   outpw(GC_INDEX, ((short)pix << 8) | SET_RESET_INDEX);
   outpw(SEQ_INDEX, ((short)pmask << 8) | MAP_MASK_INDEX);

/* calculate bitmap transfer constants */
   byteWidth = (w+7)/8;
   rShift = (x % 8);
   lShift = 8 - rShift;
   mask = (unsigned char)(0xFF << rShift);

/* write bitmap to screen */
   for(ly=0; ly<h; ly++)
   {
      offset = (ly+y)*80 + (x/8);
      bitmapIndex = ly * byteWidth;
      if(!rShift)                   /* special case, no shifting req. */
      {
         for(lx=0; lx<byteWidth; lx++)
            screen[offset++] &= bitmap[bitmapIndex++];
      }
      else
      {
         screenByte = 0;
         for(lx=0; lx<byteWidth; lx++)
         {
            screenByte |= (bitmap[bitmapIndex] & mask) >> rShift;
            screen[offset++] &= screenByte;
            screenByte = (bitmap[bitmapIndex++] & ~mask) << lShift;
         }
         screen[offset] &= screenByte;
      }
   }

/* reset read/write mode */
   outpw(GC_INDEX, (originalGCMode << 8) | GC_MODE_INDEX);
   outpw(GC_INDEX, 0x0F00 | COLOR_DONT_CARE);
   
/* unlock the task */
   unlockTask(flag);
}

/*
 * Read back a pixel value from the screen
 */
static unsigned long far _GetPixel(unsigned short x, unsigned short y, unsigned long pmask)
{
unsigned short i, offset;
unsigned char pixel, bitmask;

/* Calculate byte offset & bitmask in video buffer */
   offset = y*80+(x/8);
   bitmask = 0x80 >> (x%8);

/* Read a byte from each bit plane into memory, shift into pixel value */
   pixel = 0;
   i = 3;
   do
   {
      outpw(GC_INDEX, (i << 8) | READ_MAP_INDEX);
      pixel <<= 1;
      if(screen[offset] & bitmask)
         pixel |= 1;
   }
   while(i-- > 0);

/* Now apply pixel mask */
   return (unsigned long)pixel & pmask;
}

static void far _ChangeColormap(unsigned short ncols, cmap_t far *cols)
{
unsigned short i, lp, hp;
short flag;

   flag = lockTask();
   lp = 16;
   hp = 0;
   for(i=0; i<ncols; i++)
   {
      lp = min(lp, (short)cols[i].pixel);
      hp = max(hp, (short)cols[i].pixel);
      cmap[(short)cols[i].pixel] = cols[i].rgb;
   }
   setDACregisters(&cmap[lp], lp, hp-lp+1);
   unlockTask(flag);
}

/* End */
