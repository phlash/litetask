#include <stdio.h>
#include <dos.h>
#include <graph.h>

typedef struct {
      int n_lines;
      int n_bytes;
      char data[10][2];
      } object_t;

typedef long palette_t[256];

void drawshape(object_t *pobj, int x, int y, int planemask, int colour);

object_t obj = { 10, 2, { { 0x03, 0xC0 },
                         { 0x07, 0xE0 },
                         { 0x3F, 0xFC },
                         { 0xff, 0xff },
                         { 0xf3, 0xCf },
                         { 0xff, 0xff },
                         { 0xff, 0xff },
                         { 0x3F, 0xFC },
                         { 0x07, 0xE0 },
                         { 0x03, 0xC0 } }
                         };

palette_t palette[2];

void main(int argc, char *argv[])
{
int i, colour, x, y, mask;

/* read starting colour (or assume 0) */
   if(argc > 1)
      colour = atoi(argv[2]);
   else
      colour = 0;

/* set video mode (320x200, 256 colour) */
   _setvideomode(_MRES256COLOR);

/* set up palette0 to have white in pixel cells 0-1, black elsewhere */
   for(i=0; i<2; i++)
      palette[0][i] = _BLACK;
   for(i=2; i<256; i++)
      palette[0][i] = _WHITE;

/* set up palette1 to have white in pixel cells 0 & 2, black elsewhere */
   palette[1][0] = _BLACK;
   palette[1][1] = _WHITE;
/* say press a key.. */
   puts("Press a key..");
   getch();
   x=160;
   y=100;
   for(mask=1; !(mask & 0x80); mask=mask<<1)
   {
      drawshape(&obj, x, y, mask, 0xFF);
      getch();
   }

/* reset graphics controller and die */
   putchar('\007');
   getch();
   _setvideomode(_DEFAULTMODE);
}


void drawshape(object_t *pobj, int x, int y, int planemask, int colour)
{
int offset, line, byte, pixel, skip;
char far *screen;
   
/* set up display pointer */
   FP_SEG(screen) = 0xa000;
   FP_OFF(screen) = 0;

/* display shape */
      offset = y*320 + x;
      skip = 320 - pobj->n_bytes * 8;
      for(line=0; line<pobj->n_lines; line++)
      {
         for(byte=0; byte<pobj->n_bytes; byte++)
         {
            for(pixel=0; pixel<8; pixel++)
            {
               if(pobj->data[line][byte] & (0x80>>pixel))
                  screen[offset] |= (planemask & colour);
               offset++;
            }
         }
         offset += skip;
      }
}

