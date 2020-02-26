/*------------------------------------------------------------------------
   DEBUG.H - LiteTask debugging support

   $Author:$
   $Date:$
   $Revision:$

------------------------------------------------------------------------*/

/* The global debug function pointer */
extern void (far *debugFunPtr)(char far *);

/* The global debug variable */
extern int debugFlags;

/* The debugging statement */
#define LT_DBG(type, string) { if( ((type) & debugFlags) == (type) ) \
                                    (*debugFunPtr)(string); }

/*
 * Bitfield values for type, used to select source of debug info & level of
 * information.
 */

/* Debug source bits */
#define DBG_SRC_KERNEL  0x0001
#define DBG_SRC_CLIB    0x0002
#define DBG_SRC_IOSYS   0x0004
#define DBG_SRC_LTASK   (DBG_SRC_KERNEL | DBG_SRC_CLIB | DBG_SRC_IOSYS)

#define DBG_SRC_USR0    0x0010
#define DBG_SRC_USR1    0x0020
#define DBG_SRC_USR2    0x0040
#define DBG_SRC_USR3    0x0080
#define DBG_SRC_USR4    0x0100
#define DBG_SRC_USR5    0x0200
#define DBG_SRC_USR6    0x0400
#define DBG_SRC_USR7    0x0800

#define DBG_SRC_MASK    0x0FFF

/* Debug information level bits */
#define DBG_TYPE_ERROR  0x8000
#define DBG_TYPE_EVENT  0x4000
#define DBG_TYPE_TRACE  0x2000

#define DBG_TYPE_MASK   0xF000

/* Typical combinations */
#define DBG_KERN_ERROR  (DBG_SRC_KERNEL | DBG_TYPE_ERROR)
#define DBG_KERN_EVENT  (DBG_SRC_KERNEL | DBG_TYPE_EVENT)
/* You cannot trace the kernel :) */

#define DBG_CLIB_ERROR  (DBG_SRC_CLIB | DBG_TYPE_ERROR)
#define DBG_CLIB_EVENT  (DBG_SRC_CLIB | DBG_TYPE_EVENT)
#define DBG_CLIB_TRACE  (DBG_SRC_CLIB | DBG_TYPE_TRACE)

#define DBG_IOSYS_ERROR (DBG_SRC_IOSYS | DBG_TYPE_ERROR)
#define DBG_IOSYS_EVENT (DBG_SRC_IOSYS | DBG_TYPE_EVENT)
#define DBG_IOSYS_TRACE (DBG_SRC_IOSYS | DBG_TYPE_TRACE)

#define DBG_USR0_ERROR  (DBG_SRC_USR0 | DBG_TYPE_ERROR)
#define DBG_USR0_EVENT  (DBG_SRC_USR0 | DBG_TYPE_EVENT)
#define DBG_USR0_TRACE  (DBG_SRC_USR0 | DBG_TYPE_TRACE)

/* The right way to change the pointer :) */
extern void far setDebug(void (far *newDebug)(char far *));

/* Got a pointer to dump? */
extern char far * far formatHex(void far *);

/* End */
