#include <stdio.h>
#include <fcntl.h>

static int compdata[20000];
static int compindx=0;

void main(int argc, char **argv)
{
int width, height, x, y, i;
long next, last;

   if(argc < 3)
      return;
   width = atoi(argv[1]);
   height= atoi(argv[2]);
   setmode(1, O_BINARY);
   for(y=0; y<height; y++)           // compress each line (1-D compression)
   {
      scanf("%ld,\n", &last);        // initialise last pixel value & counter
      i = 1;
      for(x=1; x<width; x++)             // for each pixel value..
      {
         scanf("%ld,\n", &next);
         if(next == last)                // if it hasn't changed..
            i++;                             // increment counter
         else                                // otherwise..
         {
            compdata[compindx++] = i;        // store counter & pixel value..
            compdata[compindx++] = (int)last;
            last = next;                     // store new pixel value & ..
            i = 1;                           // clear counter.
         }
      }
      if(i)                               // if we have not done it yet..
      {
         compdata[compindx++] = i;        // store last pixel value
         compdata[compindx++] = (int)last;
      }
   }
   write(1, &width, sizeof(int));
   write(1, &height, sizeof(int));
   write(1, &compindx, sizeof(int));
   write(1, compdata, compindx*sizeof(int));
}

