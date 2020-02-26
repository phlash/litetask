#include <dos.h>
#include <malloc.h>
#include "vgadraw.h"

#define NULL 0

#define WIDTH  64
#define HEIGHT 48

void main()
{
unsigned int i, j;
union REGS regs;
char *bitmap1, *bitmap2, *bitmap3, *bitmap4;

   regs.x.ax = 18;
   int86(0x10, &regs, &regs);

   bitmap1 = malloc( WIDTH/8*HEIGHT );
   if(bitmap1 == NULL)
      return;
   bitmap2 = malloc( WIDTH/8*HEIGHT );
   if(bitmap2 == NULL)
      return;
   bitmap3 = malloc( WIDTH/8*HEIGHT );
   if(bitmap3 == NULL)
      return;
   bitmap4 = malloc( WIDTH/8*HEIGHT );
   if(bitmap4 == NULL)
      return;
   for ( i=0; i < (WIDTH/8*HEIGHT);
      bitmap1[i]=0x55, 
      bitmap2[i]=0x33,
      bitmap3[i]=0x0F,
      bitmap4[i++]=((i & 1) ? 0xFF : 0) );
   for(i=0; i*WIDTH<640; i++)
      for(j=0; j*HEIGHT<480; j++) {
      // draw bitmap
      DrawBitmap(i*WIDTH, j*HEIGHT, WIDTH, HEIGHT, bitmap1, 1, DRAW_PLANES);
      DrawBitmap(i*WIDTH, j*HEIGHT, WIDTH, HEIGHT, bitmap2, 2, DRAW_PLANES);
      DrawBitmap(i*WIDTH, j*HEIGHT, WIDTH, HEIGHT, bitmap3, 4, DRAW_PLANES);
      DrawBitmap(i*WIDTH, j*HEIGHT, WIDTH, HEIGHT, bitmap4, 8, DRAW_PLANES);
   }
   getch();
   for(i=0; i*WIDTH<640; i++)
      for(j=0; j*HEIGHT<480; j++) {
      // clear bitmap
      DrawBitmap(i*WIDTH, j*HEIGHT, WIDTH, HEIGHT, bitmap1, 1, CLEAR_PLANES);
      DrawBitmap(i*WIDTH, j*HEIGHT, WIDTH, HEIGHT, bitmap2, 2, CLEAR_PLANES);
      DrawBitmap(i*WIDTH, j*HEIGHT, WIDTH, HEIGHT, bitmap3, 4, CLEAR_PLANES);
      DrawBitmap(i*WIDTH, j*HEIGHT, WIDTH, HEIGHT, bitmap4, 8, CLEAR_PLANES);
   }
   getch();
   regs.x.ax = 3;
   int86(0x10, &regs, &regs);
}

