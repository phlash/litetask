/* ELLIPSE.C */

#include <dos.h>
#include <graph.h>

#define GRAPH_MODE 0x0012
#define TEXT_MODE  0x0003

main()
{
int XRadius, YRadius, Color, i;
union REGS regs;

/* select graphics mode */
   _setvideomode(GRAPH_MODE);

/* draw a sequence of ellipses */
   for(i=0; i<2; i++)
   {
      for(XRadius = 319, YRadius = 1, Color = 7; YRadius < 240;
          XRadius -= 2, YRadius += 2)
      {
         DrawEllipse(640/2, 480/2, XRadius, YRadius, Color);
         Color = (Color + 1) & 0x0F;
      }
   }

/* wait for a key */
   getch();
   _setvideomode(_DEFAULTMODE);
}

