/* DRAWBITS.C */

#include <dos.h>
#include "vgadraw.h"

#define ISVGA 1

#ifdef __TURBOC__
#define outpw outportw
#endif

#define SCREEN_WIDTH_IN_BYTES   80
#define SCREEN_SEGMENT          0xA000
#define GC_INDEX                0x3CE
#define SEQ_INDEX               0x3C4
#define SET_RESET_INDEX         0
#define SET_RESET_ENABLE_INDEX  1
#define GC_MODE_INDEX           5
#define COLOR_DONT_CARE         7
#define BIT_MASK_INDEX          8
#define MAP_MASK_INDEX          2

void DrawBitmap(int X, int Y, int Width, int Height,
                  char *Bits, int ColorOrPlane, int Mode)
{
int x, y, ByteWidth;
register unsigned int ScreenIndex, BitmapIndex, LShift, RShift;
register unsigned char Mask, ScreenByte;
unsigned char far *Screen, OriginalGCMode;

/* set drawing colour, or plane */
   outpw(GC_INDEX, (0x0F << 8) | SET_RESET_ENABLE_INDEX);
   if(Mode == DRAW_PLANES)
   {
      outpw(SEQ_INDEX, (ColorOrPlane << 8) | MAP_MASK_INDEX);
      outpw(GC_INDEX, (0x0F << 8) | SET_RESET_INDEX);
   }
   else if(Mode == CLEAR_PLANES)
   {
      outpw(SEQ_INDEX, (ColorOrPlane << 8) | MAP_MASK_INDEX);
      outpw(GC_INDEX, (0x00 << 8) | SET_RESET_INDEX);
   }
   else
      outpw(GC_INDEX, (ColorOrPlane << 8) | SET_RESET_INDEX);

#if ISVGA
/* use write mode 3, read mode 1: if VGA being used */
   outp(GC_INDEX, GC_MODE_INDEX);
   OriginalGCMode = inp(GC_INDEX+1);
   outp(GC_INDEX+1, OriginalGCMode | 0x0B);

/* set color don't care to zero, ensures 0xFF is always read from VGA */
   outpw(GC_INDEX, (0x00 << 8) | COLOR_DONT_CARE);
   
/* set bit mask to 0xFF, writes to VGA will now affect all bits */
   outpw(GC_INDEX, (0xFF << 8) | BIT_MASK_INDEX);
#else
/* point at bit mask register if NOT using write mode 3 */
   outp(GC_INDEX, BIT_MASK_INDEX);
#endif

/* set screen address */
   FP_SEG(Screen) = SCREEN_SEGMENT;
   FP_OFF(Screen) = 0;

/* calculate bit block transfer constants */
    ByteWidth = (Width+7)/8;
    RShift = (X % 8);
    LShift = 8 - RShift;
    Mask = 0xFF << RShift;

/* write bitmap to screen */
   for(y=0; y<Height; y++)
   {
      ScreenIndex = (y+Y) * SCREEN_WIDTH_IN_BYTES + X/8;
      BitmapIndex = y * ByteWidth;
      if(!RShift)                   /* special case, no shifting req. */
      {
         for(x=0; x<ByteWidth; x++)
         {
#if ISVGA
            Screen[ScreenIndex++] &= Bits[BitmapIndex++];
#else
            outp(GC_INDEX+1, Bits[BitmapIndex++]);
            Screen[ScreenIndex++] |= 0xFF;
#endif
         }
      }
      else
      {
         ScreenByte = 0;
         for(x=0; x<ByteWidth; x++)
         {
            ScreenByte |= (Bits[BitmapIndex] & Mask) >> RShift;
#if ISVGA
            Screen[ScreenIndex++] &= ScreenByte;
#else
            outp(GC_INDEX+1, ScreenByte);
            Screen[ScreenIndex++] |= 0xFF;
#endif
            ScreenByte = (Bits[BitmapIndex++] & ~Mask) << LShift;
         }
#if ISVGA
         Screen[ScreenIndex] &= ScreenByte;
#else
         outp(GC_INDEX+1, ScreenByte);
         Screen[ScreenIndex] |= 0xFF;
#endif

      }
   }

/* reset colour, bit plane & write mode */
#if ISVGA
   outpw(GC_INDEX, (OriginalGCMode << 8) | GC_MODE_INDEX);
   outpw(GC_INDEX, (0x0F << 8) | COLOR_DONT_CARE);
#else
   outp(GC_INDEX+1, 0xFF);
#endif
   if(Mode != DRAW_COLOUR)
      outpw(SEQ_INDEX, (0x0F << 8) | MAP_MASK_INDEX);
   outpw(GC_INDEX, (0x00 << 8) | SET_RESET_ENABLE_INDEX);
   outpw(GC_INDEX, (0x00 << 8) | SET_RESET_INDEX);
}

void DrawPixmap(int X, int Y, int Width, int Height, char *Pixmap, int Mode)
{
int x, y, ByteWidth;
register unsigned char Bitmask;
register unsigned int ScreenIndex, PixmapIndex;
unsigned char far *Screen, OriginalGCMode;

/* use write mode 2, read mode 1 */
   outp(GC_INDEX, GC_MODE_INDEX);
   OriginalGCMode = inp(GC_INDEX+1);
   outp(GC_INDEX+1, OriginalGCMode | 0x0A);

/* set color don't care to zero, ensures 0xFF is always read from VGA */
   outpw(GC_INDEX, (0x00 << 8) | COLOR_DONT_CARE);
   
/* select bitmask register */
   outp(GC_INDEX, BIT_MASK_INDEX);

/* set screen address */
   FP_SEG(Screen) = SCREEN_SEGMENT;
   FP_OFF(Screen) = 0;

/* calculate pixmap transfer constants */
    ByteWidth = (Width+1)/2;

/* write pixmap to screen */
   for(y=0; y<Height; y++)
   {
      ScreenIndex = (y+Y) * SCREEN_WIDTH_IN_BYTES + X/8;
      PixmapIndex = y * ByteWidth;
      Bitmask = 0x80 >> (X % 8);
      for(x=0; x<Width; x++)
      {
         outp(GC_INDEX+1, Bitmask);
         Screen[ScreenIndex] &= (x & 1) ? Pixmap[PixmapIndex++] : 
                                             Pixmap[PixmapIndex] >> 4;
         if( Bitmask & 1 )
         {
            Bitmask = 0x80;
            ScreenIndex++;
         }
         else
            Bitmask >>= 1;
      }
   }

/* reset bit plane & write mode */
   outp(GC_INDEX+1, 0xFF);
   outpw(GC_INDEX, (OriginalGCMode << 8) | GC_MODE_INDEX);
   outpw(GC_INDEX, (0x0F << 8) | COLOR_DONT_CARE);
}


