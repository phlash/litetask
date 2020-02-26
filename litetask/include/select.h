/*------------------------------------------------------------------------
   SELECT.H - LiteTask device selection types

   $Author:   Phlash  $
   $Date:   27 May 1994 20:51:34  $
   $Revision:   1.1  $

------------------------------------------------------------------------*/
#ifndef _SELECT_H
#define _SELECT_H

/* The select structure for device drivers */
typedef struct selectInfo_tag {
                  struct selectInfo_tag far *next;
                  struct selectInfo_tag far *prev;
                  int type;
                  taskHandle task;
                  } selectInfo_t;

/* select types */
#define SEL_READ  0
#define SEL_WRITE 1

/* The select function */
extern int far select(int type, int nDevs, ... );

#endif
/* End */
