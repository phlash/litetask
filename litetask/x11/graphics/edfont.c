#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <signal.h>

static unsigned char far *screen = (unsigned char far *)0xA0000000;
static unsigned char font[256][8];
static int done=0;

#define Y_OFF  68
#define X_OFF  128
#define DIG_X  304

void interrupt far sigTrap(void)
{
   done = 1;
}

void drawBlock(int sx, int sy, int pix)
{
int x, y;

   for(y=0; y<8; y++)
      for(x=0; x<8; x++)
         screen[(sy+y)*320+(sx+x)] = (unsigned char)pix;
}

void drawBox(int sx, int sy, int pix)
{
int off;

   for(off=0; off<8; off++)
   {
      screen[(sy+off)*320+(sx)] = (unsigned char)pix;
      screen[(sy+off)*320+(sx+7)] = (unsigned char)pix;
      screen[(sy)*320+(sx+off)] = (unsigned char)pix;
      screen[(sy+7)*320+(sx+off)] = (unsigned char)pix;
   }
}

void drawChar(int sx, int sy, int ch, int pix)
{
int x, y;

   for(y=0; y<8; y++)
      for(x=0; x<8; x++)
         screen[sx+x+(sy+y)*320] = (font[ch][y] & (0x01 << x)) ? 15 : 0;
}

void drawBigChar(int sx, int sy, int ch, int pix)
{
int x, y;

   for(y=0; y<8; y++)
      for(x=0; x<8; x++)
         if(font[ch][y] & (0x01 << x))
            drawBlock(sx+8*x, sy+8*y, 7);
         else
            drawBlock(sx+8*x, sy+8*y, 0);
}

void displayChar(int ch)
{
char buf[3];

   drawChar(0, 0, ch, 15);
   drawBigChar(X_OFF, Y_OFF, ch, 7);
   sprintf(buf, "%02X", ch);
   drawChar(DIG_X, 0, buf[0], 15);
   drawChar(DIG_X+8, 0, buf[1], 15);
}

void main(int argc, char **argv)
{
int ff, c, ch, x, y;
union REGS regs;
void (interrupt far *oldTrap)();

// check args
   if(argc < 2)
   {
      puts("Usage: edfont <font file>");
      return;
   }

// Trap signal for exit
   oldTrap = _dos_getvect(0x1B);
   _dos_setvect(0x1B, sigTrap);

// Open font file
   if((ff = open(argv[1], O_RDWR | O_BINARY)) == -1)
   {
      fprintf(stderr, "Cannot open font file %s\n", argv[1]);
      return;
   }

// Read font file into memory
   for(c=0; c<256; c++)
      if(read(ff, font[c], 8) != 8)
      {
         perror("Reading font file");
         return;
      }

// Switch to graphics mode
   regs.x.ax = 19;
   int86(0x10, &regs, &regs);

// Draw edit border
   for(c=0; c<65; c++)
   {
      screen[(Y_OFF-1+c)*320+(X_OFF-1)] =  8;
      screen[(Y_OFF-1+c)*320+(X_OFF+64)] = 8;
      screen[(Y_OFF-1)*320+(X_OFF-1+c)] =  8;
      screen[(Y_OFF+64)*320+(X_OFF-1+c)] = 8;
   }

// Edit loop
   c = 1;
   x = y = 0;
   while(!done)
   {
      if(!c)
      {
         drawBox(X_OFF+8*x, Y_OFF+8*y, (screen[y*320+x]) ? 7 : 0);
         switch(regs.h.ah)
         {
         case 82:       /* INS - Set pixel */
            font[ch][y] |= (0x01 << x);
            screen[y*320+x] = 15;
            drawBlock(X_OFF+8*x, Y_OFF+8*y, 7);
            break;

         case 83:       /* DEL - Clear pixel */
            font[ch][y] &= ~(0x01 << x);
            screen[y*320+x] = 0;
            drawBlock(X_OFF+8*x, Y_OFF+8*y, 0);
            break;

         case 75:       /* left arrow */
            if(x) x--;
            break;
         case 77:       /* right arrow */
            if(x < 7) x++;
            break;
         case 72:       /* up arrow */
            if(y) y--;
            break;
         case 80:       /* down arrow */
            if(y < 7) y++;
            break;
         case 71:       /* Home - Display charater 0 */
            displayChar(ch=0);
            break;
         case 79:       /* End - Display character 255 */
            displayChar(ch=255);
            break;
         case 73:       /* Page up - Previous character */
            if(ch) --ch;
            displayChar(ch);
            break;
         case 81:       /* Page down - Next character */
            if(ch<255) ++ch;
            displayChar(ch);
            break;
         }
         drawBox(X_OFF+8*x, Y_OFF+8*y, 12);
      }
      else
      {
      // Display character, small & large scale
         displayChar(ch=c);

      // Display cursor
         x = y = 0;
         drawBox(X_OFF, Y_OFF, 12);
      }

   // Read keystroke using BIOS
      regs.x.ax = 0;
      int86(0x16, &regs, &regs);
      c = regs.h.al & 0xFF;
   }

// Restore text mode
   regs.x.ax = 3;
   int86(0x10, &regs, &regs);

// Save file
   lseek(ff, 0L, SEEK_SET);
   for(c=0; c<256; c++)
      write(ff, font[c], 8);

// Unhook signal
   _dos_setvect(0x1B, oldTrap);

// close file
   close(ff);
}

