/* VGADRAW.H */

#define DRAW_COLOUR  0
#define DRAW_PLANES  1
#define CLEAR_PLANES 2

void DrawBitmap(int X, int Y, int Width, int Height,
                  char *Bits, int ColorOrPlane, int Mode);

void DrawPixmap(int X, int Y, int Width, int Height,
                  char *Pixmap, int Mode);
