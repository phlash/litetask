#ifdef VGA256
 #undef VGA256
#endif
#define VGA256  0x0013
#ifdef TEXTMODE
 #undef TEXTMODE
#endif
#define TEXTMODE 0x0003
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#define RGB(r,g,b) ((((RGB_type)(b))<<16)+((RGB_type)(g)<<8)+((RGB_type)(r)))

typedef struct
{
   unsigned char  red;
   unsigned char  green;
   unsigned char  blue;
}  RGB_type;

typedef struct
{
   RGB_type colour[256];
}  Palette_type;

extern void setVGAMode(int mode);
extern void BiosSetDACregisters(Palette_type *pal);
extern void setDACregisters(Palette_type *pal);
extern void cyclePalette(Palette_type *palette,int no_cycles, int delay, int cycle_size);
extern void plotPixel(int x, int y, int color);
extern void DrawHorizon(int x, int y, int color, int length);
extern Palette_type ColorTable;
