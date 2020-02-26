/*------------------------------------------------------------------------
   SERIAL.H - LiteTask Hardware level serial port driver

   $Author:   Phlash  $
   $Date:   27 May 1994 20:49:34  $
   $Revision:   1.2  $

------------------------------------------------------------------------*/

#include "select.h"

/* Maximum number of serial ports */
#define MAX_SERIALPORTS 2

/* IOCTL's for serial port, require arg=&(int) */
#define SERIOCGETBAUD   0
#define SERIOCSETBAUD   1
#define SERIOCGETSTATUS 2

/* Serial port status values */

#define SER_RXDATA      0x01     /* Line status register in lower byte */
#define SER_OVERRUN     0x02
#define SER_PARITY      0x04
#define SER_FRAMING     0x08
#define SER_BREAK       0x10
#define SER_THRE        0x20
#define SER_TSRE        0x40

#define SER_DCTS        0x0100   /* Modem status register in upper byte */
#define SER_DDSR        0x0200
#define SER_TERI        0x0400
#define SER_DRLSD       0x0800
#define SER_CTS         0x1000
#define SER_DSR         0x2000
#define SER_RI          0x4000
#define SER_RLSD        0x8000

#define SER_ERRBITS     0x001E   /* Bits which indicate an error */


/* 8250/16450 control register values */

#define COM_LCR_DIV     0x80     /* Line Control Register */
#define COM_LCR_BRK     0x40
#define COM_LCR_SPAR    0x20
#define COM_LCR_EPAR    0x10
#define COM_LCR_PAREN   0x08
#define COM_LCR_STOP    0x04
#define COM_LCR_5BITS   0x00
#define COM_LCR_6BITS   0x01
#define COM_LCR_7BITS   0x02
#define COM_LCR_8BITS   0x03

#define COM_IIR_NOINT   0x01     /* Interrupt ID register */
#define COM_IIR_INTID   0x06
#define COM_IIR_MODEM   0x00
#define COM_IIR_TXRDY   0x02
#define COM_IIR_RXDATA  0x04
#define COM_IIR_STAT    0x06

#define COM_IER_RXDATA  0x01     /* Interrupt Enable Register */
#define COM_IER_TXRDY   0x02
#define COM_IER_STATUS  0x04
#define COM_IER_MODEM   0x08

#define COM_MCR_DTR     0x01     /* Modem Control Register */
#define COM_MCR_RTS     0x02
#define COM_MCR_OUT1    0x04
#define COM_MCR_OUT2    0x08     /* NB: This is used to gate interrupts! */
#define COM_MCR_LOOP    0x10

/* Serial port modes */

/* Choose one of these basic port configs */
#define SER_MODE_8N1    COM_LCR_8BITS
#define SER_MODE_8O1    (COM_LCR_8BITS | COM_LCR_PAREN)
#define SER_MODE_8E1    (COM_LCR_8BITS | COM_LCR_PAREN | COM_LCR_EPAR)

/* OR in these control flags, or use ioctl() calls after creating device */
#define SER_MODE_ECHO   0x0100
#define SER_MODE_BLK    0x0200
#define SER_MODE_ASYNC  0x0400

/* driver data structures */
#define SERIAL_BUF_SIZE 256
typedef struct {
            int portId;
            int baseAddr;
            int trap;
            int mode;
            int baud;
            int status;
            int pendWhy;
            taskHandle pendTask;
            selectInfo_t far *selList;
            void far *oldTrap;
            int head;
            int tail;
            unsigned char buffer[SERIAL_BUF_SIZE];
            } serialData_t;

/* available functions */
extern int far installSerial(void);
extern int far createSerial(int portId, int mode);
extern int far deleteSerial(int devId);
extern int far removeSerial(void);

/* End */
