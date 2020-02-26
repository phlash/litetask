#include <graph.h>

void DrawEllipse(int X, int Y, int XRadius, int YRadius, int Color)
{
   _setcolor(Color);
   _ellipse(_GBORDER,X-XRadius,Y-YRadius,X+XRadius,Y+YRadius);
}

