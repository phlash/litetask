#include "litetask.h"

int mainStackSize = MINSTACK;

#define PMHEIGHT  9
#define PMWIDTH   9

unsigned long Pixmap[PMHEIGHT][PMWIDTH] =
{ { 12, 12, 12, 10, 10, 10, 9, 9, 9 },
  { 12, 12, 12, 10, 10, 10, 9, 9, 9 },
  { 12, 12, 12, 10, 10, 10, 9, 9, 9 },
  { 10, 10, 10, 9, 9, 9, 12, 12, 12 },
  { 10, 10, 10, 9, 9, 9, 12, 12, 12 },
  { 10, 10, 10, 9, 9, 9, 12, 12, 12 },
  { 9, 9, 9, 12, 12, 12, 10 ,10, 10 },
  { 9, 9, 9, 12, 12, 12, 10 ,10, 10 },
  { 9, 9, 9, 12, 12, 12, 10 ,10, 10 } };

#define BMHEIGHT  12
#define BMWIDTH   12

unsigned char Bitmap[BMHEIGHT][(BMWIDTH+7)/8] =
{ { 0x00, 0x00 },
  { 0x07, 0x00 },
  { 0x0F, 0x80 },
  { 0x0C, 0xC0 },
  { 0x18, 0xC0 },
  { 0x1D, 0xE0 },
  { 0x1F, 0xE0 },
  { 0x18, 0xE0 },
  { 0x30, 0x60 },
  { 0x70, 0x60 },
  { 0x60, 0xC0 },
  { 0x00, 0x00 } };

void far RunTest(primsDriver_t far *drv)
{
unsigned short i, len, x, y;
unsigned long pmask;
cmap_t color;
char msg[20];

/* Set pixel mask */
   pmask = (1L << drv->planes) - 1L;

/* Draw two points */
   drv->DrawPoint(drv->width-1, 0, 15L, pmask);
   drv->DrawPoint(drv->width-1, drv->height-1, 15L, pmask);
   delayTask(18L);

/* Draw a set of diagonal lines */
   for(i=0; i<drv->height; i+= 10)
      drv->DrawLine(0, 0, drv->width/2, i, 9L, pmask);
   for(i=0; i<drv->width/2; i+= 10)
      drv->DrawLine(0, 0, i, drv->height-1, 9L, pmask);
   delayTask(18L);

/* Draw two rectangles */
   drv->DrawRectangle(10, 10, drv->width/2-10, drv->height-10, 14L, pmask);
   drv->FillRectangle(drv->width/2+10, 10,
                      drv->width-10, drv->height-10, 8L, pmask);
   delayTask(18L);

/* Tile a bitmap */
   for(y=drv->height/3; y<(drv->height*2)/3; y+= BMHEIGHT)
      for(x=drv->width/8; x<(drv->width*3)/8; x += BMWIDTH)
         drv->DrawBitmap(x, y, BMWIDTH, BMHEIGHT,
            (unsigned char far *)Bitmap, 5L, pmask);
   delayTask(18L);

/* Tile a Pixmap */
   for(y=drv->height/3; y<(drv->height*2)/3; y+= PMHEIGHT)
      for(x=(drv->width*5)/8; x<(drv->width*7)/8; x += PMWIDTH)
         drv->DrawPixmap(x, y, PMWIDTH, PMHEIGHT,
             (unsigned long far *)Pixmap, pmask);
   delayTask(18L);

/* Draw a text string */
   for(len=0; drv->description[len]; len++);
   drv->DrawString(0, 20, drv->description, len, 7L, pmask);
   delayTask(18L);

/* cycle a colour */
   color.pixel = 8L;
   for(color.rgb=0L; color.rgb<0x40; color.rgb++)
      drv->ChangeColormap(1, &color);
}

void far mainTask(char far *com)
{
char msg[80];
unsigned short i;

/* Dump data */
   printk(vm18drv.description);
   printk("\r\n");
   sprintf(msg, "vm18drv: width=%i height=%i planes=%i\r\n",
      vm18drv.width, vm18drv.height, vm18drv.planes);
   printk(msg);
   printk(vm19drv.description);
   printk("\r\n");
   sprintf(msg, "vm19drv: width=%i height=%i planes=%i\r\n",
      vm19drv.width, vm19drv.height, vm19drv.planes);
   printk(msg);

/* Check command line for specific driver */
   for(i=0; com[i]; i++)
   {
      if(com[i] == '8')
      {
      /* Initialise mode 18 driver */
         printk("mainTask(): Initialising video mode 18\r\n");
         vm18drv.InitDriver();

      /* Run test in this mode */
         printk("mainTask(): Running test\r\n");
         RunTest(&vm18drv);
         printk("mainTask(): Delaying..\r\n");
         delayTask(90L);
         vm18drv.RemoveDriver();
      }
      if(com[i] == '9')
      {
      /* Initialise mode 19 driver */
         printk("mainTask(): Initialising video mode 19\r\n");
         vm19drv.InitDriver();

      /* Run test in this mode */
         printk("mainTask(): Running test\r\n");
         RunTest(&vm19drv);
         printk("mainTask(): Delaying..\r\n");
         delayTask(90L);
         vm19drv.RemoveDriver();
      }
   }
}
       
/* End */
