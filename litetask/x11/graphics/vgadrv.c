/*------------------------------------------------------------------------
   VGADRV.C - VGA 640x480 mode display driver, supports pixel operations

   $Author:   Phlash  $
   $Date:   25 Nov 1993 11:50:30  $
   $Revision:   1.4  $

------------------------------------------------------------------------*/

#include "litetask.h"

#define BIOS_TEXT_MODE           3
#define BIOS_GRAPH_MODE          18

#define GRAPH_BASE               0xA000
#define GRAPH_WIDTH              640
#define GRAPH_HEIGHT             480
#define GC_INDEX                 0x3CE
#define SEQ_INDEX                0x3C4
#define SCREEN_WIDTH_IN_BYTES    80
#define SET_RESET_INDEX          0
#define SET_RESET_ENABLE_INDEX   1
#define GC_MODE_INDEX            5
#define COLOR_DONT_CARE          7
#define BIT_MASK_INDEX           8
#define MAP_MASK_INDEX           2

#define ISVGA 1

/* Routines in MISCGLUE.ASM */
extern void far outp(int port, unsigned char byte);
extern void far outpw(int port, unsigned short word);
extern unsigned char far inp(int port);
extern unsigned short far inpw(int port);

/*------------------------------------------------------------------------
   changemode - Set video mode to VGA 640x480, or 80x25 colour text
------------------------------------------------------------------------*/
void far changemode(int mode)
{
union REGS regs;
short flag;

   flag = lockTask();
   switch(mode)
   {
   case TEXT_MODE:
      regs.x.ax = BIOS_TEXT_MODE;
      break;
   case GRAPH_MODE:
      regs.x.ax = BIOS_GRAPH_MODE;
      break;
   default:
      unlockTask(flag);
      return;
   }
   int86(0x10, &regs);
   unlockTask(flag);
}

/*------------------------------------------------------------------------
   setpixel - Draw one pixel in a specified colour
------------------------------------------------------------------------*/
void far setpixel(int x, int y, int colour)
{
int offset;
unsigned char bitmask, far *screen;
short flag;

/* lock task */
   flag = lockTask();

/* set color */
   outpw(GC_INDEX, 0x0F00 | SET_RESET_ENABLE_INDEX);
   outpw(GC_INDEX, (colour << 8) | SET_RESET_INDEX);
   outp(GC_INDEX, BIT_MASK_INDEX);

/* set pixel to colour value */
   offset = y * (GRAPH_WIDTH/8) + x/8;
   bitmask = (unsigned char)(0x80 >> (x%8));
   FP_SEG(screen) = GRAPH_BASE;
   FP_OFF(screen) = offset;
   outp(GC_INDEX+1, bitmask);
   *screen |= 0xFF;

/* reset bitmask & colour */
   outp(GC_INDEX+1, 0xFF);
   outpw(GC_INDEX, 0x0000 | SET_RESET_ENABLE_INDEX);

/* unlock task */
   unlockTask(flag);
}

/*------------------------------------------------------------------------
   drawBitmap - set a bitplane, or draw a single colour bitmap
------------------------------------------------------------------------*/
void far drawBitmap(int x, int y, int width, int height,
                  char far *bitmap, int colorOrPlane, int mode)
{
int lx, ly, byteWidth;
register unsigned int screenIndex, bitmapIndex, lShift, rShift;
register unsigned char mask, screenByte;
unsigned char far *screen, originalGCMode;
short flag;

/* lock task */
   flag = lockTask();

/* set drawing colour, or plane */
   outpw(GC_INDEX, (0x0F << 8) | SET_RESET_ENABLE_INDEX);
   if(mode == DRAW_PLANES)
   {
      outpw(SEQ_INDEX, (colorOrPlane << 8) | MAP_MASK_INDEX);
      outpw(GC_INDEX, (0x0F << 8) | SET_RESET_INDEX);
   }
   else if(mode == CLEAR_PLANES)
   {
      outpw(SEQ_INDEX, (colorOrPlane << 8) | MAP_MASK_INDEX);
      outpw(GC_INDEX, (0x00 << 8) | SET_RESET_INDEX);
   }
   else
      outpw(GC_INDEX, (colorOrPlane << 8) | SET_RESET_INDEX);

#if ISVGA
/* use write mode 3, read mode 1: if VGA being used */
   outp(GC_INDEX, GC_MODE_INDEX);
   originalGCMode = inp(GC_INDEX+1);
   outp(GC_INDEX+1, originalGCMode | 0x0B);

/* set color don't care to zero, ensures 0xFF is always read from VGA */
   outpw(GC_INDEX, (0x00 << 8) | COLOR_DONT_CARE);
   
/* set bit mask to 0xFF, writes to VGA will now affect all bits */
   outpw(GC_INDEX, (0xFF << 8) | BIT_MASK_INDEX);
#else
/* point at bit mask register if NOT using write mode 3 */
   outp(GC_INDEX, BIT_MASK_INDEX);
#endif

/* set screen address */
   FP_SEG(screen) = GRAPH_BASE;
   FP_OFF(screen) = 0;

/* calculate bit block transfer constants */
    byteWidth = (width+7)/8;
    rShift = (x % 8);
    lShift = 8 - rShift;
    mask = (unsigned char)(0xFF << rShift);

/* write bitmap to screen */
   for(ly=0; ly<height; ly++)
   {
      screenIndex = (ly+y) * SCREEN_WIDTH_IN_BYTES + x/8;
      bitmapIndex = ly * byteWidth;
      if(!rShift)                   /* special case, no shifting req. */
      {
         for(lx=0; lx<byteWidth; lx++)
         {
#if ISVGA
            screen[screenIndex++] &= bitmap[bitmapIndex++];
#else
            outp(GC_INDEX+1, bitmap[bitmapIndex++]);
            screen[screenIndex++] |= 0xFF;
#endif
         }
      }
      else
      {
         screenByte = 0;
         for(lx=0; lx<byteWidth; lx++)
         {
            screenByte |= (bitmap[bitmapIndex] & mask) >> rShift;
#if ISVGA
            screen[screenIndex++] &= screenByte;
#else
            outp(GC_INDEX+1, screenByte);
            screen[screenIndex++] |= 0xFF;
#endif
            screenByte = (bitmap[bitmapIndex++] & ~mask) << lShift;
         }
#if ISVGA
         screen[screenIndex] &= screenByte;
#else
         outp(GC_INDEX+1, screenByte);
         screen[screenIndex] |= 0xFF;
#endif

      }
   }

/* reset colour, bit plane & write MODE */
#if ISVGA
   outpw(GC_INDEX, (originalGCMode << 8) | GC_MODE_INDEX);
   outpw(GC_INDEX, (0x0F << 8) | COLOR_DONT_CARE);
#else
   outp(GC_INDEX+1, 0xFF);
#endif
   if(mode != DRAW_COLOUR)
      outpw(SEQ_INDEX, (0x0F << 8) | MAP_MASK_INDEX);
   outpw(GC_INDEX, (0x00 << 8) | SET_RESET_ENABLE_INDEX);
   outpw(GC_INDEX, (0x00 << 8) | SET_RESET_INDEX);
   
/* unlock the task */
   unlockTask(flag);
}

/*------------------------------------------------------------------------
   drawPixmap - draw a multi-colour bitmap (pixmap)
------------------------------------------------------------------------*/
void far drawPixmap(int x, int y, int width, int height,
                     char far *pixmap, int mode)
{
int lx, ly, byteWidth;
register unsigned char bitmask;
register unsigned int screenIndex, pixmapIndex;
unsigned char far *screen, originalGCMode;
short flag;

/* lock the task */
   flag = lockTask();

/* use write mode 2, read mode 1 */
   outp(GC_INDEX, GC_MODE_INDEX);
   originalGCMode = inp(GC_INDEX+1);
   outp(GC_INDEX+1, originalGCMode | 0x0A);

/* set color don't care to zero, ensures 0xFF is always read from VGA */
   outpw(GC_INDEX, (0x00 << 8) | COLOR_DONT_CARE);
   
/* select bitmask register */
   outp(GC_INDEX, BIT_MASK_INDEX);

/* set screen address */
   FP_SEG(screen) = GRAPH_BASE;
   FP_OFF(screen) = 0;

/* calculate pixmap transfer constants */
   byteWidth = (width+1)/2;

/* write pixmap to screen */
   for(ly=0; ly<height; ly++)
   {
      screenIndex = (ly+y) * SCREEN_WIDTH_IN_BYTES + x/8;
      pixmapIndex = ly * byteWidth;
      bitmask = (unsigned char)(0x80 >> (x % 8));
      for(lx=0; lx<width; lx++)
      {
         outp(GC_INDEX+1, bitmask);
         screen[screenIndex] &= (lx & 1) ? pixmap[pixmapIndex++] : 
                                             pixmap[pixmapIndex] >> 4;
         if( bitmask & 1 )
         {
            bitmask = 0x80;
            screenIndex++;
         }
         else
            bitmask >>= 1;
      }
   }

/* reset bit plane & write MODE */
   outp(GC_INDEX+1, 0xFF);
   outpw(GC_INDEX, (originalGCMode << 8) | GC_MODE_INDEX);
   outpw(GC_INDEX, (0x0F << 8) | COLOR_DONT_CARE);

/* unlock the task */
   unlockTask(flag);
}

/* End */
