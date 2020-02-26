#include <graph.h>
#include <bios.h>

void main(void)
{
int width, height, repeat, pixel, i;

   _setvideomode(_MRES256COLOR);
   scanf("/* width=%d height=%d */\n", &width, &height);
   i = 0;
   while(i<width*height)
   {
      scanf("%d, %d,\n", &repeat, &pixel);
      _setcolor(pixel);
      _moveto(i%width, height-i/width);
      _lineto(i%width+repeat, height-i/width);
      i+= repeat;
   }
   _bios_keybrd(_KEYBRD_READ);
   _setvideomode(_DEFAULTMODE);
}

