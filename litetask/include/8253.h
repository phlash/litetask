/*------------------------------------------------------------------------
   8253.H - Intel 8253 Timer definitions for IBM PC

   $Author:$
   $Date:$
   $Revision:$

------------------------------------------------------------------------*/

/* 8253 port addresses for the PC */
#define TMR_CNTL_PORT      0x43
#define TMR0_CNTR_PORT     0x40
#define TMR1_CNTR_PORT     0x41
#define TMR2_CNTR_PORT     0x42

/* Control port bits */
#define TMR0_CNTL          0x00           /* Program timer x */
#define TMR1_CNTL          0x40
#define TMR2_CNTL          0x80

#define TMR_LATCH          0x00           /* Latch current count */
#define TMR_LBYTE          0x10           /* read/load low byte */
#define TMR_HBYTE          0x20           /* read/load high bytes */
#define TMR_WORD           0x30           /* read/load full word */
#define TMR_MODE0          0x00           /* int on TC mode */
#define TMR_MODE1          0x02           /* prog one-shot mode */
#define TMR_MODE2          0x04           /* rate gen. mode */
#define TMR_MODE3          0x06           /* square wave gen. mode */
#define TMR_MODE4          0x08           /* s/w triggered strobe */
#define TMR_MODE5          0x0A           /* h/w triggered strobe */
#define TMR_BINARY         0x00           /* binary counter */
#define TMR_BCD            0x01           /* BCD (4 decade) counter */

/* End */
