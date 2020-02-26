#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>

static unsigned char far *screen = (unsigned char far *)0xA0000000;

void main(int argc, char **argv)
{
int ff, c, x, y;
union REGS regs;
unsigned char bitmap[8];

// check args
   if(argc < 2)
   {
      puts("Usage: genfont <font file>");
      return;
   }

// Open font file
   if((ff = open(argv[1], O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0666)) == -1)
   {
      printf("Cannot create font file %s\n", argv[1]);
      return;
   }

// Switch to graphics mode
   regs.x.ax = 19;
   int86(0x10, &regs, &regs);

// Draw each character and save the bitmap to file
   for(c=0; c<256; c++)
   {
   // Draw on screen using BIOS (no processing by DOS!!)
      regs.x.ax = 0x0A00 + c;
      regs.x.bx = 0x0007;
      regs.x.cx = 1;
      int86(0x10, &regs, &regs);

   // Grab the screen bits and save to file
      for(y=0; y<8; y++)
      {
         bitmap[y] = 0;
         for(x=0; x<8; x++)
            if(screen[y*320+x])
               bitmap[y] |= (1 << x);
      }
      write(ff, bitmap, 8);
      for(y=0; y<8; y++)
         for(x=0; x<8; x++)
            screen[y*320+x] = 0;
   }

// Restore text mode
   regs.x.ax = 3;
   int86(0x10, &regs, &regs);

// close file
   close(ff);
}

