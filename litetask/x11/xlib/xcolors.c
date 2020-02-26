/*------------------------------------------------------------------------
   XCOLORS.C - LiteTask X11 Library, color handling routines

   $Author:$
   $Date:$
   $Revision:$

------------------------------------------------------------------------*/

#include "litetask.h"
#include "Xlib.h"
#include "Xint.h"
#include "colors.h"

/* Local data */
static colormap_t _cmap;        // The default Colormap

/* Internal functions */

int far XIInstallColormap(void)
{
int i;

/* Create and install the default colormap */
   _cmap.size = 1 << X11pDrv->planes;
   _cmap.installed = 1;
   _cmap.next = NULL;
   _cmap.map = (cmap_t far *)malloc( sizeof(cmap_t) * _cmap.size );
   _cmap.flags = (char far *)malloc( sizeof(char) * _cmap.size );
   if(!_cmap.map || !_cmap.flags)
   {
      return False;
   }
   for(i=0; i<_cmap.size; i++)
   {
      _cmap.map[i].pixel = i;
      if(i < _MAXBIOS_PIX)
      {
         _cmap.map[i].rgb = _BIOSCOLOR(i);
         _cmap.flags[i] = _READ_ONLY;
      }
      else
      {
         _cmap.map[i].rgb = _BLACK;
         _cmap.flags[i] = _AVAILABLE;
      }
   }
   X11pDrv->ChangeColormap(_cmap.size, _cmap.map);
   return True;
}

void far XIRemoveColormap(void)
{
   free(_cmap.map);
   free(_cmap.flags);
}

Colormap far XIDefaultColormap(void)
{
   return (Colormap)&_cmap;
}

/* External functions */

int far XAllocColorCells(Display far *dpy, Colormap cmap, int contig,
                     unsigned long far *planes, int nplanes,
                     unsigned long far *pixels, int npixels)
{
colormap_t far *pcmap = (colormap_t far *)cmap;
int ncells, cell, nextcell, blocksize, plane, pixel;
short flag;
char buf[80];

/* check arguments */
   if(npixels < 1 || nplanes < 0)
   {
      sprintf(buf, "XAllocColorCells: invalid args: planes=%i pixels=%i\r\n",
            nplanes, npixels);
      printk(buf);
      return False;
   }

/* see if request is possible, ie: total entries required <= colormap depth */
   blocksize = 1 << nplanes;
   if((ncells=npixels * blocksize) > pcmap->size)
   {
      printk("XAllocColorCells: too many cells requested\r\n");
      return False;
   }

/*
   Find first free cell block which fits on 2^nplanes boundary, and can hold 
   all color cells required.
   
   NB: This is contiguous allocation, I can't be bothered with frags yet!
*/
   flag = lockTask();
   nextcell = 0;
   do
   {
   /* start at next boundary */
      cell = ( (nextcell + blocksize - 1)/blocksize ) * blocksize;

   /* search for a free cell */
      while(cell<pcmap->size && pcmap->flags[cell] != _AVAILABLE)
         cell += blocksize;
   
   /* if we ran out of colormap, then no can do.. */
      if(cell >= pcmap->size)
      {
         unlockTask(flag);
         printk("XAllocColorCells: ran out of colormap\r\n");
         return False;
      }
   
   /* now find the next unavailable cell, or the end of the map */
      for(nextcell=cell; nextcell<pcmap->size && pcmap->flags[nextcell] == _AVAILABLE; nextcell++)
         ;
   } while((nextcell-cell) < ncells);

/* Found a hole big enough, so allocate cells... */

/* write pixel cell values */
   for(pixel=0; pixel<npixels; pixel++)
      pixels[pixel] = (unsigned long)(cell+pixel*blocksize);

/* write plane masks */
   for(plane=0; plane<nplanes; plane++)
   {
      planes[plane] = (unsigned long)(1 << plane);
   }

/* mark cells as allocated */
   for(nextcell=0; nextcell<ncells; nextcell++)
      pcmap->flags[cell+nextcell] = _ALLOCATED;

/* all done (phew!) */
   unlockTask(flag);
   return True;
}

int far XAllocColor(Display far *dpy, Colormap cmap, XColor far *xc)
{
colormap_t far *pcmap = (colormap_t far *)cmap;
unsigned long tmp, distance=2000000L;
int i, match=-1;
char buf[80];

/* scan colormap for closest _READ_ONLY RGB value to that requested */
/* currently a bodge since it should compare Red, Green & Blue seperately */

   for(i=0; i<_MAXBIOS_PIX; i++)
   {
      if((tmp = Abs(xc->rgb - pcmap->map[i].rgb)) < distance)
      {
         distance = tmp;
         match = i;
      }
   }
   if(match == -1)
   {
      sprintf(buf, "XAllocColor: weird color value %l!\r\n", xc->rgb);
      return False;
   }
   xc->pixel = (unsigned long)match;
   xc->rgb   = pcmap->map[match].rgb;
   return True;
}

int far XParseColor(Display far *dpy, Colormap cmap, char far *color, XColor far *xc)
{
char buf[80];

/* see if 'color' is one of the standard VGA colours */
   if(memcmp(color, "black", 5) == 0)
      xc->rgb = _BLACK;
   else if(memcmp(color, "blue", 4) == 0)
      xc->rgb = _BLUE;
   else if(memcmp(color, "green", 5) == 0)
      xc->rgb = _GREEN;
   else if(memcmp(color, "cyan", 4) == 0)
      xc->rgb = _CYAN;
   else if(memcmp(color, "red", 3) == 0)
      xc->rgb = _RED;
   else if(memcmp(color, "magenta", 7) == 0)
      xc->rgb = _MAGENTA;
   else if(memcmp(color, "brown", 5) == 0)
      xc->rgb = _BROWN;
   else if(memcmp(color, "lightgray", 9) == 0)
      xc->rgb = _LIGHTGRAY;
   else if(memcmp(color, "gray", 4) == 0)
      xc->rgb = _GRAY;
   else if(memcmp(color, "lightblue", 9) == 0)
      xc->rgb = _LIGHTBLUE;
   else if(memcmp(color, "lightgreen", 10) == 0)
      xc->rgb = _LIGHTGREEN;
   else if(memcmp(color, "lightcyan", 9) == 0)
      xc->rgb = _LIGHTCYAN;
   else if(memcmp(color, "lightred", 8) == 0)
      xc->rgb = _LIGHTRED;
   else if(memcmp(color, "lightmagenta", 12) == 0)
      xc->rgb = _LIGHTMAGENTA;
   else if(memcmp(color, "lightyellow", 11) == 0)
      xc->rgb = _LIGHTYELLOW;
   else if(memcmp(color, "white", 5) == 0)
      xc->rgb = _WHITE;
   else
   {
      sprintf(buf, "XParseColor: invalid color name %s\r\n", color);
      printk(buf);
      return False;
   }
  
  return True;
}

int far XStoreColor(Display far *dpy, Colormap cmap, XColor xc)
{
colormap_t far *pcmap = (colormap_t far *)cmap;
short flag;
char buf[80];

/* check cell is allocated */
   if(xc.pixel<0L || xc.pixel>255L)
   {
      sprintf(buf, "XStoreColor: cell out of range: %l\r\n", xc.pixel);
      printk(buf);
      return False;
   }
   flag = lockTask();
   if(pcmap->flags[(int)xc.pixel] != _ALLOCATED)
   {
      unlockTask(flag);
      sprintf(buf, "XStoreColor: cell %l not allocated\r\n", xc.pixel);
      printk(buf);
      return False;
   }
   pcmap->map[(int)xc.pixel].rgb = xc.rgb;

/* update hardware if this is an installed map */
   if(pcmap->installed)
      X11pDrv->ChangeColormap(1, &(pcmap->map[(int)xc.pixel]));
   unlockTask(flag);
   return True;
}

int far XStoreColors(Display far *dpy, Colormap cmap, XColor far *xc, int ncolors)
{
colormap_t far *pcmap = (colormap_t far *)cmap;
int i;
short flag;
char buf[80];

/* Check args */
   if(ncolors > pcmap->size)
   {
      sprintf(buf, "XStoreColors: too many colors %i\r\n", ncolors);
      printk(buf);
      return False;
   }

/* Check color allocation */
   flag = lockTask();
   for(i=0; i<ncolors; i++)
   {
      if(xc[i].pixel<0L || xc[i].pixel>(long)(pcmap->size-1))
      {
         unlockTask(flag);
         sprintf(buf, "XStoreColors: cell out of range (%l) at offset %i\r\n",
               xc[i].pixel, i);
         printk(buf);
         return False;
      }
      if(pcmap->flags[(int)xc[i].pixel] != _ALLOCATED)
      {
         unlockTask(flag);
         sprintf(buf, "XStoreColors: cell %l not allocated at offset %i\n",
               xc[i].pixel, i);
         printk(buf);
         return False;
      }
   }

/* now change colormap and update hardware if installed */
   for(i=0; i<ncolors; i++)
      pcmap->map[(int)xc[i].pixel].rgb = xc[i].rgb;
   if(pcmap->installed)
      X11pDrv->ChangeColormap(pcmap->size, pcmap->map);
   unlockTask(flag);
   return True;
}

/* End */
