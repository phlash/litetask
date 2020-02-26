/*------------------------------------------------------------------------
   TEXTDRV.C - Text mode display driver, supports character and string O/P

   $Author:   Phlash  $
   $Date:   07 Apr 1995 20:53:28  $
   $Revision:   1.10  $

------------------------------------------------------------------------*/

#include "litetask.h"

#define CGA_BASE     0xb8000000L
#define MDA_BASE     0xb0000000L
#define PAGESIZE     (textDrv.width * 2 * 25)
#define DEF_WIDTH    80
#define DEF_HEIGHT   25
#define DEF_ATTRIB   0x0700

/* Local prototypes */
static int  far InitText(void);
static void far RemoveText(void);
static void far OutChar(int c);
static void far OutString(char far *str);
static void far OutCharXY(int x, int y, int c);
static void far OutStringXY(int x, int y, char far *str);
static void far ClearText(void);
static void far ScrollText(int lines);
static int  far CtrlText(int ctrl, void far *arg);

/* The driver interface */
textDriver_t textDrv = {
"LiteTask CGA/MDA text driver $Revision:   1.10  $, copyright (AshbySoft *) 1995",
                     DEF_WIDTH, DEF_HEIGHT,
                     InitText,
                     RemoveText,
                     OutChar,
                     OutString,
                     OutCharXY,
                     OutStringXY,
                     ClearText,
                     ScrollText,
                     CtrlText
                     };

static char SPACES[] = "                                                                                ";
static int far *textScreen = NULL;
static int curMode = -1, oldMode = -1;
static int textX = 0, textY = 0;

int far InitText(void)
{
union REGS regs;
short flag;

/* read current video mode and set screen size / address information */
   flag = lockTask();
   if(curMode >= 0)
   {
      unlockTask(flag);
      return curMode;
   }
   regs.x.ax = 0x0F00;
   int86(0x10, &regs);
   switch(regs.h.al)
   {
   case 0:                 /* 40x25 CGA B&W */
   case 1:                 /* 40x25 CGA Col */
   case 2:                 /* 80x25 CGA B&W */
   case 3:                 /* 80x25 CGA Col */

   /* All these are standard CGA modes, just use them */
      oldMode = (int)regs.h.al;
      curMode = oldMode;
      textDrv.width = (int)regs.h.ah;
      textScreen = (int far *)CGA_BASE;
      break;

   case 4:                 /* 320x200 CGA graphics Col */
   case 5:                 /* 320x200 CGA graphics B&W */
   case 6:                 /* 640x200 CGA graphics B&W */
   case 8:                 /* 160x200 PCjr graphics */
   case 9:                 /* 320x200 PCjr graphics */
   case 10:                /* 640x200 PCjr graphics */
   case 13:                /* 320x200 EGA graphics */
   case 14:                /* 640x200 EGA graphics */
   case 15:                /* 640x350 EGA graphics (2 col) */
   case 16:                /* 640x350 EGA graphics (4/16 col) */
   case 17:                /* 640x480 MCGA graphics */
   case 18:                /* 640x480 VGA graphics */
   case 19:                /* 320x200 MCGA graphics */

   /* All these can do CGA compatible text mode 3, so set it */
      oldMode = (int)regs.h.al;
      curMode = 3;
      textScreen = (int far *)CGA_BASE;
      regs.x.ax = curMode;
      int86(0x10, &regs);
      break;

   case 7:                 /* 80x25 MDA B&W */

   /* This is an MDA, and must be used in this mode */
      oldMode = (int)regs.h.al;
      curMode = oldMode;
      textScreen = (int far *)MDA_BASE;
      break;

   case 11:
   case 12:
   default:

   /* We give up and print a string using the BIOS (which may know how to) */
      biosStr("InitText: unknown video adapter/mode!\r\n");
      unlockTask(flag);
      return -1;
   }

/* make sure we are on display page 0 */
   regs.x.ax = 0x0500;
   int86(0x10, &regs);

/* read current cursor position */
   regs.x.ax = 0x0300;
   regs.x.bx = 0;
   int86(0x10, &regs);
   textX = (int)regs.h.dl;
   textY = (int)regs.h.dh;
   unlockTask(flag);
   return curMode;
}

void far RemoveText(void)
{
union REGS regs;
short flag;

/* Put back the original video mode if we changed it */
   flag = lockTask();
   if(curMode != oldMode)
   {
      regs.h.ah = 0;
      regs.h.al = (unsigned char)oldMode;
      int86(0x10, &regs);
      curMode = oldMode = -1;
   }
   unlockTask(flag);
}

void far OutChar(int c)
{
short flag;

   flag = lockTask();
   if(curMode == -1)
   {
      biosCh(c);
      unlockTask(flag);
      return;
   }
   switch(c)
   {
   case '\r':
      textX = 0;
      break;
   case '\n':
      textY++;
      if(textY >= textDrv.height)
      {
         textY--;
         ScrollText(1);
      }
      break;
   case '\f':
      textX = textY = 0;
      ClearText();
      break;
   default:
      OutCharXY(textX, textY, c);
      textX++;
      if(textX >= textDrv.width)
      {
         textX = 0;
         textY++;
         if(textY >= textDrv.height)
         {
            textY--;
            ScrollText(1);
         }
      }
      break;
   }
   unlockTask(flag);
}

void far OutString(char far *string)
{
short flag;
int offset;

/* display one char at a time, interpret CR,LF,FF chars */
   flag = lockTask();
   if(curMode == -1)
   {
      biosStr(string);
      unlockTask(flag);
      return;
   }
   offset = textY * textDrv.width;
   while(*string)
   {
      switch(*string)
      {
      case '\r':
         textX = 0;
         break;
      case '\n':
         textY++;
         if(textY >= textDrv.height)
         {
            textY--;
            ScrollText(1);
         }
         offset = textY * textDrv.width;
         break;
      case '\f':
         textX = textY = 0;
         offset = 0;
         ClearText();
         break;
      default:
         textScreen[offset+textX] = (*string & 0xFF) | DEF_ATTRIB;
         textX++;
         if(textX >= textDrv.width)
         {
            textX = 0;
            textY++;
            if(textY >= textDrv.height)
            {
               textY--;
               ScrollText(1);
            }
            offset = textY * textDrv.width;
         }
         break;
      }
      string++;
   }
   unlockTask(flag);
}

void far OutCharXY(int x, int y, int c)
{
int offset;

/* place character at specified location */
   if(curMode != -1)
   {
      offset = y * textDrv.width + x;
      textScreen[offset] = (c & 0xFF) | DEF_ATTRIB;
   }
}

void far OutStringXY(int x, int y, char far *string)
{
int offset;

/* draw string, starting at x, y */
   if(curMode != -1)
   {
      offset = y * textDrv.width + x;
      while(*string)
      {
         textScreen[offset++] = ((int)(*string) & 0xFF) | DEF_ATTRIB;
         string++;
      }
   }
}

void far ClearText(void)
{
int line;

/* fill screen with spaces */
   for(line = 0; line < textDrv.height; line++)
      OutStringXY(0, line, SPACES);
}

void far ScrollText(int lines)
{
int x, y;

/* check installed */
   if(curMode == -1)
      return;

/* scroll the contents of the screen */
   if(lines < 0)
   {
      for(y=textDrv.height-1; y>= -lines; y--)
      {
         for(x=0; x<textDrv.width; x++)
            textScreen[y*textDrv.width+x] = textScreen[(y+lines)*textDrv.width+x];
      }
      for(;y >= 0; y--)
         OutStringXY(0, y, SPACES);
   }
   else
   {
      for(y=0; y<textDrv.height-lines; y++)
      {
         for(x=0; x<textDrv.width; x++)
            textScreen[y*textDrv.width+x] = textScreen[(y+lines)*textDrv.width+x];
      }
      for(;y<textDrv.height; y++)
         OutStringXY(0, y, SPACES);
   }
}

int far CtrlText(int ctrl, void far *arg)
{
union REGS regs;
short flag;

/* check installed */
   if(curMode == -1)
      return -1;

   flag = lockTask();
   switch(ctrl)
   {
   case TEXT_CTRL_RESET:
      textX = textY = 0;
      regs.x.ax = curMode;
      int86(0x10, &regs);
      break;
   case TEXT_CTRL_CGASCREEN:
      if(curMode == 7)
      {
         textX = textY = 0;
         textScreen = (int far *)CGA_BASE;
         curMode = 3;
         regs.x.ax = curMode;
         int86(0x10, &regs);
      }
      break;
   case TEXT_CTRL_MDASCREEN:
      if(curMode != 7)
      {
         textX = textY = 0;
         textScreen = (int far *)MDA_BASE;
         curMode = 7;
         regs.x.ax = curMode;
         int86(0x10, &regs);
      }
      break;
   default:
      OutString("CtrlText: invalid driver control!\r\n");
      unlockTask(flag);
      return -1;
   }
   unlockTask(flag);
   return 0;
}

/* End. */
