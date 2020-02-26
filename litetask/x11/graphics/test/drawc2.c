#include <graph.h>

void DrawCircle(int X, int Y, int Radius, int Color)
{
   _setcolor(Color);
   _ellipse(_GBORDER,X-Radius,Y-Radius,X+Radius,Y+Radius);
}

