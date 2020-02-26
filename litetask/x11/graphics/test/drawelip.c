/* DRAWELIP.C */

#include <dos.h>

#define ISVGA 0

#ifdef __TURBOC__
#define outpw outportw
#endif

#define SCREEN_WIDTH_IN_BYTES   80
#define SCREEN_SEGMENT          0xA000
#define GC_INDEX                0x3CE
#define SET_RESET_INDEX         0
#define SET_RESET_ENABLE_INDEX  1
#define GC_MODE_INDEX           5
#define COLOR_DONT_CARE         7
#define BIT_MASK_INDEX          8

unsigned char PixList[SCREEN_WIDTH_IN_BYTES*8/2];

void DrawEllipse(int X, int Y, int A, int B, int Color)
{
int Length;
long Threshold;
long ASquared = (long) A * A;
long BSquared = (long) B * B;
long XAdjust, YAdjust;
unsigned char *PixListPtr, OriginalGCMode;

/* set drawing color */
   outpw(GC_INDEX, (0x0F << 8) | SET_RESET_ENABLE_INDEX);
   outpw(GC_INDEX, (Color << 8) | SET_RESET_INDEX);

#if ISVGA
   outp(GC_INDEX, GC_MODE_INDEX);
   OriginalGCMode = inp(GC_INDEX+1);
   outp(GC_INDEX+1, OriginalGCMode | 0x0B);
   outpw(GC_INDEX, (0x00 << 8) | COLOR_DONT_CARE);
   outpw(GC_INDEX, (0xFF << 8) | BIT_MASK_INDEX);
#else
   outp(GC_INDEX, BIT_MASK_INDEX);
#endif

   Length = GenerateEOctant(PixList, (long) ASquared * 2 * B,
            (long) ASquared / 4L - ASquared * B, ASquared, BSquared);
   
   DrawHOctant(X,Y-B,Length,SCREEN_WIDTH_IN_BYTES,0,PixList);
   DrawHOctant(X+1,Y-B-(*PixList),Length-1,SCREEN_WIDTH_IN_BYTES,1,PixList+1);
   DrawHOctant(X,Y+B,Length,-SCREEN_WIDTH_IN_BYTES,0,PixList);
   DrawHOctant(X+1,Y+B-(*PixList),Length-1,-SCREEN_WIDTH_IN_BYTES,1,PixList+1);

   Length = GenerateEOctant(PixList, (long) BSquared * 2 * A,
            (long) BSquared / 4L - BSquared * A, BSquared, ASquared);

   DrawVOctant(X-A,Y,Length,-SCREEN_WIDTH_IN_BYTES,1,PixList);
   DrawVOctant(X-A+(*PixList),Y+1,Length-1,SCREEN_WIDTH_IN_BYTES,1,PixList+1);
   DrawVOctant(X+A,Y,Length,-SCREEN_WIDTH_IN_BYTES,0,PixList);
   DrawVOctant(X+A-(*PixList),Y+1,Length-1,SCREEN_WIDTH_IN_BYTES,0,PixList+1);

#if ISVGA
   outpw(GC_INDEX, (OriginalGCMode << 8) | GC_MODE_INDEX);
   outpw(GC_INDEX, (0x0F << 8) | COLOR_DONT_CARE);
#else
   outp(GC_INDEX+1, 0xFF);
#endif
   outpw(GC_INDEX, (0x00 << 8) | SET_RESET_ENABLE_INDEX);
}

