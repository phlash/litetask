/*------------------------------------------------------------------------
   BMP.H - MS-Windows .BMP file format structures

   $Author:$
   $Date:$
   $Revision:$

------------------------------------------------------------------------*/

/* On-disk structures */
#pragma pack(1)
typedef struct {
            WORD  bfType;
            DWORD bfSize;
            WORD  bfResv1, bfResv2;
            DWORD bfOffsetBits;
            } BMPFILEHEADER;

typedef struct {
            DWORD biSize;
            DWORD biWidth;
            DWORD biHeight;
            WORD  biPlanes;
            WORD  biBitsPerPixel;
            DWORD biCompression;
            DWORD biImageSize;
            DWORD biXPelsPerMetre;
            DWORD biYPelsPerMetre;
            DWORD biColors;
            DWORD biImpColors;
            } BMPINFOHEADER;

typedef struct {
            BMPFILEHEADER bf;
            BMPINFOHEADER bi;
            } BMPFILE;

typedef struct {
            BYTE blue, green, red, resv;
            } RGBQUAD;

#pragma pack()

/* In-memory structures */
typedef struct {
            DWORD bcSize;
            DWORD bcWidth;
            DWORD bcHeight;
            WORD  bcPlanes;
            WORD  bcBitsPerPixel;
            } BMPINFOCORE;

typedef struct {
            BMPINFOCORE bc;
            cmap_t far *cmap;
            long far * far *bitmap;     // Held in normalised 'Pixmap' form
            } BMP;

/* End */
