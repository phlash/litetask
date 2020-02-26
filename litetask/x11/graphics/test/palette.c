#include <dos.h>
#include <conio.h>

#define GC_INDEX  0x3CE
#define SET_RESET_INDEX 0
#define SET_RESET_ENABLE_INDEX   1
#define BIT_MASK_INDEX  8

static unsigned char oldVDAC[256][3];
static unsigned long VDAC[256];
static unsigned char oldpal[17], pal[17] =
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0 };

static unsigned char far *screen = (unsigned char far *)0xA0000000;

static void setDACregister(int pixel, unsigned long rgb)
{
/* wait for vSync */
   while(!(inp(0x3DA) & 0x08));
   _disable();
   outp(0x3C8, pixel);
   outp(0x3C9, (short)rgb & 0x3F);
   outp(0x3C9, (short)(rgb >> 8) & 0x3F);
   outp(0x3C9, (short)(rgb >> 16) & 0x3F);
   _enable();
}

static void drawLine18(short y, short pix)
{
unsigned short i;

   outpw(GC_INDEX, 0xFF00 | SET_RESET_ENABLE_INDEX);
   outpw(GC_INDEX, (pix << 8) | SET_RESET_INDEX);
   outpw(GC_INDEX, 0xFF00 | BIT_MASK_INDEX);
   for(i=y*80; i<y*80+80; i++)
      screen[i] |= 0xFF;
}

static void drawLine19(short y, short pix)
{
unsigned short i;

   for(i=320*y; i<320*y+320; i++)
      screen[i] = pix;
}

static void dumpRegisters(void)
{
unsigned short x;
union REGS regs;
struct SREGS sregs;

/* dump current EGA palette & VideoDAC settings */
   regs.x.ax = 0x1009;
   segread(&sregs);
   sregs.es = sregs.ds;
   regs.x.dx = oldpal;
   int86x(0x10, &regs, &regs, &sregs);
   printf("EGA palette: ");
   for(x=0; x<16; x++)
      printf("%02X ", oldpal[x]);
   printf("Border: %02X\n", oldpal[16]);
   getch();   
   
   regs.x.ax = 0x1017;
   regs.x.bx = 0;
   regs.x.cx = 256;
   segread(&sregs);
   sregs.es = sregs.ds;
   regs.x.dx = oldVDAC;
   int86x(0x10, &regs, &regs, &sregs);
   printf("Video DAC: R G B,");
   for(x=0; x<256; x++)
   {
      if(!(x%8))
         printf("\n%02X: ", x);
      printf("%02X ", oldVDAC[x][0]);
      printf("%02X ", oldVDAC[x][1]);
      printf("%02X,", oldVDAC[x][2]);
   }
   getch();
}

void main(void)
{
unsigned short x, y;
union REGS regs;
struct SREGS sregs;

/* dump current settings */
   dumpRegisters();

/* set video mode */
   regs.x.ax = 18;
   int86(0x10, &regs, &regs);
   dumpRegisters();

/* set up Video DAC colormap, repeated red green and blue values */
   for(x=1; x<256; x++)
   {
      switch(x % 3)
      {
      case 0:
         VDAC[x] = 0x0000003F;
         break;
      case 1:
         VDAC[x] = 0x00003F00;
         break;

      case 2:
         VDAC[x] = 0x003F0000;
         break;
      }
   }

/* draw a set of colour bars */
   for(y=0; y<480; y++)
      drawLine18(y, (y/10)%16);
   getch();

/* set up EGA palette registers */
   segread(&sregs);
   sregs.es = sregs.ds;
   regs.x.ax = 0x1002;
   regs.x.dx = pal;
   int86x(0x10, &regs, &regs, &sregs);
   getch();
   dumpRegisters();

/* change the Video DAC colormap */
   for(x=1; x<16; x++)
      setDACregister(x, VDAC[x]);
   getch();
   dumpRegisters();

/* reset video mode */
   regs.x.ax = 3;
   int86(0x10, &regs, &regs);
}
