/*------------------------------------------------------------------------
   XUTIL.C - LiteTask X11 Library, utility routines

   $Author:$
   $Date:$
   $Revision:$

------------------------------------------------------------------------*/

#include "litetask.h"
#include "Xlib.h"
#include "Xint.h"

int far XIRectanglesOverlap(rectangle_t far *r1, rectangle_t far *r2)
{
/* If the X boundaries & the Y boundaries overlap, the rectangles overlap */
   if( ((r1->x+r1->w > r2->x) && (r1->x < r2->x+r2->w)) &&
       ((r1->y+r1->h > r2->y) && (r1->y < r2->y+r2->h)) )
       return True;
   return False;
}

/* End */
