/*------------------------------------------------------------------------
   BIOS.H - LiteTask BIOS level device drivers header

   $Author:   Phlash  $
   $Date:   28 Apr 1994 19:02:10  $
   $Revision:   1.0  $

------------------------------------------------------------------------*/

/* The drivers */
#include "bioscon.h"
#include "biosdisk.h"


/* The generic support bits */

/* BIOS multi-tasking support via INT 15h */
extern void far installInt15(void);
extern void far removeInt15(void);
extern int far checkInt15(void);

/* End */
