#include <stdio.h>
#include <conio.h>
#include <dos.h>

#define INT_CNTL_PORT   0x20
#define INT_MASK_PORT   0x21
#define EOI 0x20

static int COM_BASE[] = { 0x3F8, 0x2F8 };
#define N_PORTS 2
#define COM_TXBUF(port) (COM_BASE[(port)%N_PORTS]+0)
#define COM_RXBUF(port) (COM_BASE[(port)%N_PORTS]+0)
#define COM_DIVLO(port) (COM_BASE[(port)%N_PORTS]+0)
#define COM_DIVHI(port) (COM_BASE[(port)%N_PORTS]+1)
#define COM_IER(port) (COM_BASE[(port)%N_PORTS]+1)
#define COM_IIR(port) (COM_BASE[(port)%N_PORTS]+2)
#define COM_LCR(port) (COM_BASE[(port)%N_PORTS]+3)
#define COM_MCR(port) (COM_BASE[(port)%N_PORTS]+4)
#define COM_LSR(port) (COM_BASE[(port)%N_PORTS]+5)
#define COM_MSR(port) (COM_BASE[(port)%N_PORTS]+6)

#define SERIAL_BUF_SIZE 256

static int trapIn = 0;
static void (interrupt far *oldTrap)();
static int port = 0, trap = 0, head = 0, tail = 0;
static unsigned char serialBuf[SERIAL_BUF_SIZE];

static void interrupt far _serialTrap(void)
{
int iir, lsr;

/* see if this device interrupted */
   iir = inp(COM_IIR(port));
   if(iir & 0x01)              /* no interrupt on this device */
   {
      if(oldTrap)
         _chain_intr(oldTrap);
      return;
   }

/* clear interrupt controller */
   outp(INT_CNTL_PORT, EOI);

/* read line status */
   lsr = inp(COM_LSR(port));
   if(lsr & 0x1E)
      return;

/* read data byte into buffer */
   serialBuf[head++] = inp(COM_RXBUF(port));
   if(head >= SERIAL_BUF_SIZE)
      head = 0;
}

int removeSerial(void)
{
   outp(COM_IER(port), 0);
   outp(INT_MASK_PORT, inp(INT_MASK_PORT) | (1 << trap));
   _dos_setvect(trap+8, oldTrap);
}

int initSerial(char *device, int speed)
{
int divisor;

/* select device from name */
   switch(device[3])
   {
   case '1':
      port = 0;
      trap = 4;
      break;
   case '2':
      port = 1;
      trap = 3;
      break;
   default:
      return -1;
   }

/* Display current settings */
   printf("Initialising port COM%d to %d baud\n", port+1, speed);
   printf("IER=0x%02X, IIR=0x%02X, LCR=0x%02X, MCR=0x%02X, LSR=0x%02X, MSR=0x%02X\n",
      inp(COM_IER(port)),
      inp(COM_IIR(port)),
      inp(COM_LCR(port)),
      inp(COM_MCR(port)),
      inp(COM_LSR(port)),
      inp(COM_MSR(port)));

/* Calculate divisor from baud rate spec */
   if(speed < 50 || speed > 19200)
   {
      fprintf(stderr, "Error, invalid baud rate %d\n", speed);
      return -1;
   }
   divisor = (int) (115200L/(long)speed);

/* Set up COM port for specified baud, 8-bit, 1 stop, no parity */
   outp(COM_LCR(port), 0x80);
   outp(COM_DIVLO(port), divisor & 0xFF);
   outp(COM_DIVHI(port), ((unsigned)divisor) >> 8);
   outp(COM_LCR(port), 0x03);
   outp(COM_IER(port), 0);
   outp(COM_MCR(port), 0x03);
   
/* install interrupt trap */
   if(!trapIn)
   {
      trapIn = 1;
      atexit(removeSerial);
      oldTrap = _dos_getvect(trap+8);
      _dos_setvect(trap+8, _serialTrap);
      outp(INT_MASK_PORT, inp(INT_MASK_PORT) & ~(1 << trap));
      outp(COM_IER(port), 0x01);
   }
   return 0;
}

int readSerial(int dev, char *buf, int bytes)
{
int i;

   for(i=0; i<bytes; i++)
   {
      if(tail == head)
         break;
      buf[i] = serialBuf[tail++];
      if(tail >= SERIAL_BUF_SIZE)
         tail = 0;
   }
   return i;
}

int writeSerial(int dev, char *buf, int bytes)
{
int i;

   for(i=0; i<bytes; i++)
   {
   /* wait for THRE */
      while(!(inp(COM_LSR(port)) & 0x20))
         ;
   /* Send byte */
      outp(COM_TXBUF(port), buf[i]);
   }
   return bytes;
}

