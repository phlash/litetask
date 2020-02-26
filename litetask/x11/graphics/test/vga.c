/* VGA Driver for ASHBYSOFT Multi-tasking Kernel */

#include <dos.h>
#include "ianvga.h"
#include "vgadrv.h"
#include "video.h"

#define BIOS_TEXT_MODE  3
#define BIOS_GRAPH_MODE 18

#define TEXT_BASE    0xb800
#define TEXT_WIDTH   80
#define TEXT_HEIGHT  25
#define DEF_ATTRIB   0x0700

#define GRAPH_BASE   0xa000
#define GRAPH_WIDTH  640
#define GRAPH_HEIGHT 480
#define REG1         0x3ce
#define SET_RESET_ENABLE_INDEX 1
#define SET_RESET_INDEX 0
#define BIT_MASK_INDEX 8

void setmode(int mode);

/****** TEXT_MODE routines ******/

void outchar(int c, int x, int y)
{
int offset;
int far *screen;

/* place character at specified location */
   offset = y * TEXT_WIDTH *2 + x * 2;
   FP_SEG(screen) = TEXT_BASE;
   FP_OFF(screen) = offset;
   *screen = (c & 0xFF) | DEF_ATTRIB;
}

void outcharatt(int char_att, int x, int y)
{
int offset;
int far *screen;

/* place character and attribute at specified location */
   offset = y * TEXT_WIDTH * 2 + x * 2;
   FP_SEG(screen) = TEXT_BASE;
   FP_OFF(screen) = offset;
   *screen = char_att;
}

void outstring(char *string, int x, int y)
{
int offset;
int far *screen;

/* draw string, starting at x, y */
   offset = y * TEXT_WIDTH * 2 + x * 2;
   FP_SEG(screen) = TEXT_BASE;
   FP_OFF(screen) = offset;
   offset = 0;
   while(*string)
   {
      screen[offset++] = ((int)(*string) & 0xFF) | DEF_ATTRIB;
      string++;
   }
}

void outstringatt(int *string_att, int x, int y)
{
int offset;
int far *screen;

/* draw string, starting at x, y */
   offset = y * TEXT_WIDTH * 2 + x * 2;
   FP_SEG(screen) = TEXT_BASE;
   FP_OFF(screen) = offset;
   offset = 0;
   while(*string_att)
   {
      screen[offset++] = *string_att;
      string_att++;
   }
}

/****** GRAPH_MODE routines ******/

void setpixel(int x, int y, int colour)
{
int offset;
unsigned char bitmask, far *screen;

/* set color */
   outpw(REG1, 0x0F00 | SET_RESET_ENABLE_INDEX);
   outpw(REG1, (colour << 8) | SET_RESET_INDEX);
   outp(REG1, BIT_MASK_INDEX);

/* set pixel to colour value */
   offset = y * (GRAPH_WIDTH/8) + x/8;
   bitmask = 0x80 >> (x%8);
   FP_SEG(screen) = GRAPH_BASE;
   FP_OFF(screen) = offset;
   outp(REG1+1, bitmask);
   *screen |= 0xFF;

/* reset bitmask & colour */
   outp(REG1+1, 0xFF);
   outpw(REG1, 0x0000 | SET_RESET_ENABLE_INDEX);
}

/****** Change mode routine ******/

void changemode(int mode)
{
   if(mode == TEXT_MODE)
      setmode(BIOS_TEXT_MODE);
   else
      setmode(BIOS_GRAPH_MODE);
}

void setmode(int mode)
{
union REGS regs;

/* use INT 10h to change mode */
   regs.x.ax = mode;
   int86(0x10, &regs, &regs);
}
