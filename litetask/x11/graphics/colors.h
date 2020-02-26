/* Colors.h - RGB values for VGA/MCGA video DAC chip */

#define _BLACK          0x000000L
#define _BLUE           0x2a0000L
#define _GREEN          0x002a00L
#define _CYAN           0x2a2a00L
#define _RED            0x00002aL
#define _MAGENTA        0x2a002aL
#define _BROWN          0x00152aL
#define _LIGHTGRAY      0x2a2a2aL
#define _GRAY           0x151515L
#define _LIGHTBLUE      0x3F1515L
#define _LIGHTGREEN     0x153f15L
#define _LIGHTCYAN      0x3f3f15L
#define _LIGHTRED       0x15153fL
#define _LIGHTMAGENTA   0x3f153fL
#define _LIGHTYELLOW    0x153f3fL
#define _WHITE          0x3f3f3fL

/* Default BIOS Pixel values for these colors */
#define _BLACK_PIX      0
#define _BLUE_PIX       1
#define _GREEN_PIX      2
#define _CYAN_PIX       3
#define _RED_PIX        4
#define _MAGENTA_PIX    5
#define _BROWN_PIX      6
#define _LIGHTGRAY_PIX  7
#define _GRAY_PIX       8
#define _LIGHTBLUE_PIX  9
#define _LIGHTGREEN_PIX 10
#define _LIGHTCYAN_PIX  11
#define _LIGHTRED_PIX   12
#define _LIGHTMAGENTA_PIX 13
#define _LIGHTYELLOW_PIX  14
#define _WHITE_PIX      15

#define _MAXBIOS_PIX    16   // Largest value set in BIOS colormap

/* Nasty macro to select a color */
#define _BIOSCOLOR(pix) ( (pix == 0) ? _BLACK :       \
                          (pix == 1) ? _BLUE :        \
                          (pix == 2) ? _GREEN :       \
                          (pix == 3) ? _CYAN :        \
                          (pix == 4) ? _RED :         \
                          (pix == 5) ? _MAGENTA :     \
                          (pix == 6) ? _BROWN :       \
                          (pix == 7) ? _LIGHTGRAY :   \
                          (pix == 8) ? _GRAY :        \
                          (pix == 9) ? _LIGHTBLUE :   \
                          (pix == 10) ? _LIGHTGREEN : \
                          (pix == 11) ? _LIGHTCYAN :  \
                          (pix == 12) ? _LIGHTRED :   \
                          (pix == 13) ? _LIGHTMAGENTA:\
                          (pix == 14) ? _LIGHTYELLOW :\
                          (pix == 15) ? _WHITE :      \
                          -1L )

/* End */
