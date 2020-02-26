/* CIRCLE.C */

#include <dos.h>
#include <graph.h>

#define GRAPH_MODE 0x0012
#define TEXT_MODE  0x0003

main()
{
int Radius, Color, i;
union REGS regs;

/* select graphics mode */
   _setvideomode(GRAPH_MODE);

/* draw a sequence of circles */
   for(i=0; i<2; i++)
   {
      for(Radius = 0, Color = 7; Radius < 240; Radius += 2)
      {
         DrawCircle(640/2, 480/2, Radius, Color);
         Color = (Color + 1) & 0x0F;
      }
   }

/* wait for a key */
   getch();
   _setvideomode(_DEFAULTMODE);
}

