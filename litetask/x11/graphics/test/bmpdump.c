#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <fcntl.h>
#include <io.h>
#include <bios.h>
#include <graph.h>

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;

#pragma pack(1)
typedef struct {
            WORD  bfType;
            DWORD bfSize;
            WORD  bfResv1, bfResv2;
            DWORD bfOffsetBits;
            } BMPFILEHEADER;

typedef struct {
            DWORD biSize;
            DWORD biWidth;
            DWORD biHeight;
            WORD  biPlanes;
            WORD  biBitsPerPixel;
            DWORD biCompression;
            DWORD biImageSize;
            DWORD biXPelsPerMetre;
            DWORD biYPelsPerMetre;
            DWORD biColors;
            DWORD biImpColors;
            } BMPINFOHEADER;

typedef struct {
            BMPFILEHEADER bf;
            BMPINFOHEADER bi;
            } BMPFILE;

typedef struct {
            BYTE blue, green, red, resv;
            } RGBQUAD;

#pragma pack()

void mapColor(short pixel, RGBQUAD *color)
{
/* Map 8bits/primary to 6bits/primary (sorry, no 24-bit displays here :),
   then store in video DAC */
   _disable();
   outp(0x3C8, pixel);
   outp(0x3C9, color->red >> 2);
   outp(0x3C9, color->green >> 2);
   outp(0x3C9, color->blue >> 2);
   _enable();
}

void displayMono(char *line, short len, short y)
{
short x, idx, mask;

   _setcolor(1);
   x = 0;
   mask = 0x80;
   idx = 0;
   while(x < len)
   {
      if(line[idx] & mask)
      {
         _setpixel(x, y);
         puts("1,");
      }
      else
         puts("0,");
      x++;
      if(mask == 1)
      {
         mask = 0x80;
         idx++;
      }
      else
         mask >>= 1;
   }
}

void display16Color(char *line, short len, short y)
{
unsigned char pix;
short x, idx, mask;

   x = 0;
   mask = 0xF0;
   idx = 0;
   while(x < len)
   {
      pix = line[idx] & mask;
      if(mask == 0xF0)
      {
         pix = (pix >> 4) & 0xF;
         mask = 0x0F;
      }
      else
      {
         pix &= 0xF;
         mask = 0xF0;
         idx++;
      }
      if(pix)
      {
         _setcolor(pix);
         _setpixel(x, y);
      }
      printf("%d,\n", pix);
      x++;
   }
}

void display256Color(unsigned char *line, short len, short y)
{
short x;

   for(x=0; x<len; x++)
   {
      if(line[x])
      {
         _setcolor(line[x]);
         _setpixel(x, y);
      }
      printf("%d,\n", line[x]);
   }
}


void main(int argc, char **argv)
{
int bmpFile, nCols, i, k;
long scanLine, j;
BMPFILE info;
RGBQUAD color;
char *lineBuff;
void (*displayLine)(char *, short, short);


/* check args */
   if(argc > 1)
   {
      bmpFile = open(argv[1], O_RDONLY | O_BINARY);
      if(bmpFile < 0)
      {
         perror("Opening bitmap file:");
         return;
      }
   }
   else
   {
      setmode(0, O_BINARY);
      bmpFile = 0;
   }

/* read header off file */
   if(read(bmpFile, (char *)&info, sizeof(info)) != sizeof(info))
   {
      fprintf(stderr, "Reading bitmap file: file too short\n");
      goto Done;
   }

/* Dump info */
   fprintf(stderr, "File signature: %.2s\n", &info.bf.bfType);
   fprintf(stderr, "File size:      %ld\n", info.bf.bfSize);
   fprintf(stderr, "Offset to bits: %ld\n", info.bf.bfOffsetBits);
   fprintf(stderr, "Bitmap size:    %ld x %ld\n", info.bi.biWidth, info.bi.biHeight);
   fprintf(stderr, "Bitmap planes:  %d\n", info.bi.biPlanes);
   fprintf(stderr, "Bitmap bits/pix:%d\n", info.bi.biBitsPerPixel);
   fprintf(stderr, "Bitmap compress:%ld\n", info.bi.biCompression);

/* wait for a keypress */
   fprintf(stderr, "Press a key to continue..");
   _bios_keybrd(_KEYBRD_READ);

/* set video mode */
   switch(info.bi.biBitsPerPixel)
   {
   case 1:
      _setvideomode(_VRES2COLOR);
      displayLine = displayMono;
      break;
   case 4:
      _setvideomode(_VRES16COLOR);
      displayLine = display16Color;
      break;
   case 8:
      _setvideomode(_MRES256COLOR);
      displayLine = display256Color;
      break;
   default:
      fprintf(stderr, "Unsupported number of colors :)\n");
      goto Done;
   }

/* map RGB table */
   nCols = 1 << info.bi.biBitsPerPixel;
   for(i=0; i<nCols; i++)
   {
      if(read(bmpFile, (char *)&color, sizeof(color)) != sizeof(color))
      {
         fprintf(stderr, "Reading RGB table: file too short\n");
         goto VidDone;
      }
      mapColor(i, &color);
   }

/* display bitmap data */
   lseek(bmpFile, info.bf.bfOffsetBits, SEEK_SET);
   scanLine = (info.bi.biWidth * (long)(info.bi.biBitsPerPixel) + 31L) / 32L * 4L;
   if(scanLine > 32767L)
   {
      fprintf(stderr, "Reading bitmap: too wide for me :)\n");
      goto VidDone;
   }
   lineBuff = (char *)malloc( (short)scanLine );
   if(!lineBuff)
   {
      perror("Allocating bitmap buffer:");
      goto VidDone;
   }
   for(j=0L; j<info.bi.biHeight; j++)
   {
      if(read(bmpFile, lineBuff, (short)scanLine) != (short)scanLine)
      {
         fprintf(stderr, "Reading bitmap: file too short\n");
         goto VidDone;
      }
      displayLine(lineBuff, (short)info.bi.biWidth, (short)(info.bi.biHeight-j));
   }
   free(lineBuff);

/* wait for a keypress */
   _settextposition(24, 0);
   _settextcolor(15);
   _outtext("Press a key to continue..");
   _bios_keybrd(_KEYBRD_READ);

/* reset video mode */
VidDone:
   _setvideomode(_DEFAULTMODE);

/* done */
Done:
   if(bmpFile)
      close(bmpFile);
   else
      setmode(0, O_TEXT);
}

