#include <conio.h>
#include "vgadrv.h"
#include "video.h"

void main()
{
int i,j,ch;
char string[81];

/* select graph mode */
   changemode(GRAPH_MODE);

/* set a few pixels */
   for(i=0; i<640; i++)
   {
     for(j=0; j<480; j++)
     {
      setpixel(i, j, j % 16);
     }
   }
   getch();
   changemode(TEXT_MODE);
}

