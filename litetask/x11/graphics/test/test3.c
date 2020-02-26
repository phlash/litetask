#include <dos.h>
#include <malloc.h>
#include "vgadraw.h"

#define NULL 0

#define WIDTH  64
#define HEIGHT 48

char pixmap2[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
                   0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
                   0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
                   0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
                   0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
                   0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
                   0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
                   0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
#define PIX2W 16
#define PIX2H 8

void main()
{
unsigned int i, j;
union REGS regs;
char *pixmap;

   regs.x.ax = 18;
   int86(0x10, &regs, &regs);

//   pixmap = malloc( WIDTH/2*HEIGHT );
//   if(pixmap == NULL)
//      return;
//   for ( i=0; i < (WIDTH/2*HEIGHT);
//      pixmap[i++]=((i+1)%16) | (i%16) << 4 );
   for(i=0; i*PIX2W<640; i++)
      for(j=0; j*PIX2H<480; j++) {
      // draw pixmap
      DrawPixmap(i*PIX2W, j*PIX2H, PIX2W, PIX2H, pixmap2, 0);
   }
   getch();
   regs.x.ax = 3;
   int86(0x10, &regs, &regs);
}
