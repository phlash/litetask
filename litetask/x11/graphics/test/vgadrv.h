/* VGADRV.H - Declarations of VGA driver routines */

void outchar(int c, int x, int y);
void outcharatt(int char_att, int x, int y);
void outstring(char *string, int x, int y);
void outstringatt(int *string_att, int x, int y);
void setpixel(int x, int y, int colour);
void changemode(int mode);

/* End */
