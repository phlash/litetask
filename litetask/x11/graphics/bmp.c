/*------------------------------------------------------------------------
   BMP.C - MS-Windows .BMP format file handling

   $Author:$
   $Date:$
   $Revision:$

------------------------------------------------------------------------*/

#include "litetask.h"
#include "bmp.h"

#define SEEK_SET 0

static void near mapColor(WORD pixel, cmap_t far *cmap, RGBQUAD far *color)
{
/* Map 8bits/primary to 6bits/primary (sorry, no 24-bit displays here :),
   and store in colormap */
   cmap->pixel = (long)pixel;
   cmap->rgb = (long)(color->red >> 2) ||
               ((long)(color->green >> 2) << 8) ||
               ((long)(color->blue >> 2) << 16);
}

static void near convertMono(BYTE far *line, WORD len, long far *pixmap)
{
WORD x, idx, mask;

   x = 0;
   mask = 0x80;
   idx = 0;
   while(idx < len)
   {
      if(line[idx] & mask)
         pixmap[x] = 1L;
      else
         pixmap[x] = 0L;
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

static void near convert16Color(BYTE far *line, WORD len, long far *pixmap)
{
BYTE pix;
WORD x, idx, mask;

   x = 0;
   mask = 0xF0;
   idx = 0;
   while(idx < len)
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
      pixmap[x] = (long)pix;
      x++;
   }
}

static void near convert256Color(BYTE far *line, WORD len, long far *pixmap)
{
WORD x;

   for(x=0; x<len; x++)
      pixmap[x] = (long)line[x];
}


BMP far * far readBMPFile(int bmpFile)
{
WORD nCols, i;
DWORD scanLine, j, k;
BMPFILE info;
RGBQUAD color;
BMP far *bmp = NULL;
void (near *convertLine)(BYTE far *, short, DWORD far *);
BYTE far *scanBuff = NULL;


/* read header off file */
   if(read(bmpFile, &info, sizeof(info)) != sizeof(info))
   {
      printk("readBMPFile: file too short\r\n");
      return NULL;
   }

/* can we handle this format? */
   if(info.bi.biCompression)
   {
      printk("readBMPFile: cannot read RLE files :)\r\n");
      return NULL;
   }
   switch(info.bi.biBitsPerPixel)
   {
   case 1:
      convertLine = convertMono;
      break;
   case 4:
      convertLine = convert16Color;
      break;
   case 8:
      convertLine = convert256Color;
      break;
   case 24:                 /* We'll do 24-bit eventually, when I find one! */
   default:
      printk("readBMPFile: too many colors for me :)\r\n");
      return NULL;
   }

/* calculate scan line length */
   scanLine = (info.bi.biWidth * (DWORD)(info.bi.biBitsPerPixel) + 31L) / 32L * 4L;

/* can we handle this size? */
   if(info.bi.biWidth > 65535L || scanLine > 65535L || info.bi.biHeight > 16383L)
   {
      printk("readBMPFile: picture too big for me :)\r\n");
      return NULL;
   }

/* allocate the BMP */
   bmp = (BMP far *) malloc( sizeof(BMP) );
   if(!bmp)
      goto MemError;
   bmp->cmap = NULL;
   bmp->bitmap = NULL;

/* Copy information */
   memcpy(&bmp->bc, &info, sizeof( BMPINFOCORE ) );

/* allocate cmap */
   nCols = 1 << info.bi.biBitsPerPixel;
   bmp->cmap = (cmap_t far *) malloc( sizeof(cmap_t) * nCols );
   if(!bmp->cmap)
      goto MemError;

/* allocate scan line buffer */
   scanBuff = (BYTE far *) malloc( (WORD)scanLine );
   if(!scanBuff)
      goto MemError;

/* allocate bitmap array */
   bmp->bitmap = (long far * far *) malloc( sizeof(long far *) *
                                          (WORD)info.bi.biHeight );
   if(!bmp->bitmap)
      goto MemError;
   for(j=0L; j<info.bi.biHeight; j++)
   {
      bmp->bitmap[(WORD)j] = (long far *) malloc( sizeof(long) *
                                          (WORD)info.bi.biWidth );
      if(!bmp->bitmap[(WORD)j])
         goto MemError;
   }

/* map RGB table */
   for(i=0; i<nCols; i++)
   {
      if(read(bmpFile, (char *)&color, sizeof(color)) != sizeof(color))
         goto FileError;
      mapColor(i, &bmp->cmap[i], &color);
   }

/* convert bitmap data */
   lseek(bmpFile, info.bf.bfOffsetBits, SEEK_SET);
   for(k=info.bi.biHeight-1L; k >= 0L; k--)
   {
      if(read(bmpFile, scanBuff, (WORD)scanLine) != (WORD)scanLine)
         goto FileError;
      convertLine(scanBuff, (WORD)scanLine, bmp->bitmap[(WORD)k]);
   }
   free(scanBuff);

/* All OK */
   return bmp;

FileError:
   printk("readBMPFile: error reading file\r\n");
   goto ClearUp;

MemError:
   printk("readBMPFile: out of memory\r\n");

ClearUp:
   if(bmp)
   {
      if(bmp->bitmap)
      {
         while(j--)
            free(bmp->bitmap[(WORD)j]);
         free(bmp->bitmap);
      }
      if(bmp->cmap)
         free(bmp->cmap);
      free(bmp);
   }
   if(scanBuff)
      free(scanBuff);
   return NULL;
}

