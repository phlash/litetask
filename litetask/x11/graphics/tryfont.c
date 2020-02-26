#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>

static unsigned char far *screen = (unsigned char far *)0xA0000000;

void main(int argc, char **argv)
{
int ff, c, x, y, i, j;
union REGS regs;
unsigned char bitmap[8];

// check args
   if(argc < 2)
   {
      puts("Usage: tryfont <font file> [<string>]");
      return;
   }

// Open font file
   if((ff = open(argv[1], O_RDONLY | O_BINARY)) == -1)
   {
      printf("Cannot open font file %s\n", argv[1]);
      return;
   }

// Switch to graphics mode
   regs.x.ax = 19;
   int86(0x10, &regs, &regs);

// If a string is supplied, draw it
   if(argc > 2)
   {
      for(i=2; i<argc; i++)
      {
         for(j=0; argv[i][j]; j++)
         {
            c=argv[i][j];
            lseek(ff, (long)(8*c), SEEK_SET);
            read(ff, bitmap, 8);
            for(y=0; y<8; y++)
            {
               for(x=0; x<8; x++)
                  if(bitmap[y] & (1 << x))
                     screen[320*(y+8*i)+(x+8*j)] = 7;
            }
         }
      }
   }
   else
   {
   // Draw each character on screen
      for(c=0; c<256; c++)
      {
      // Read from file
         read(ff, bitmap, 8);

      // Draw the screen bits
         for(y=0; y<8; y++)
         {
            for(x=0; x<8; x++)
               if(bitmap[y] & (1 << x))
                  screen[320*(y+8*(c/40))+(x+8*(c%40))] = 7;
         }
      }
   }
   getch();

// Restore text mode
   regs.x.ax = 3;
   int86(0x10, &regs, &regs);

// close file
   close(ff);
}

