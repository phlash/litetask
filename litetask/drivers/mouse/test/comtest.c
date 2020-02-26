#include <stdio.h>
#include <conio.h>

static int COM_BASE[] = { 0x3F8, 0x2F8 };
#define N_PORTS 2
#define COM_TXBUF(port) (COM_BASE[(port-1)%N_PORTS]+0)
#define COM_RXBUF(port) (COM_BASE[(port-1)%N_PORTS]+0)
#define COM_DIVLO(port) (COM_BASE[(port-1)%N_PORTS]+0)
#define COM_DIVHI(port) (COM_BASE[(port-1)%N_PORTS]+1)
#define COM_IER(port) (COM_BASE[(port-1)%N_PORTS]+1)
#define COM_IIR(port) (COM_BASE[(port-1)%N_PORTS]+2)
#define COM_LCR(port) (COM_BASE[(port-1)%N_PORTS]+3)
#define COM_MCR(port) (COM_BASE[(port-1)%N_PORTS]+4)
#define COM_LSR(port) (COM_BASE[(port-1)%N_PORTS]+5)
#define COM_MSR(port) (COM_BASE[(port-1)%N_PORTS]+6)

#define MS  0
#define PC  1
void msMouse(int byte);
void pcMouse(int byte);

void main(int argc, char **argv)
{
int port, line_status, mouseType = MS;
int waitLoop = 0;
char *waitBar = "-\|/";

/* select com port to use */
   if(argc > 1)
      port = atoi(argv[1]);
   else
      port = 1;
   printf("Reading from COM%d:\n", port);
   if(argc > 2)
   {
      switch(argv[2][0])
      {
      case 'm':
      case 'M':
         mouseType = MS;
         break;
      case 'p':
      case 'P':
         mouseType = PC;
         break;
      }
   }

/* Display current settings */
   printf("IER=0x%02X, IIR=0x%02X, LCR=0x%02X, MCR=0x%02X, LSR=0x%02X, MSR=0x%02X\n",
      inp(COM_IER(port)),
      inp(COM_IIR(port)),
      inp(COM_LCR(port)),
      inp(COM_MCR(port)),
      inp(COM_LSR(port)),
      inp(COM_MSR(port)));

/* Set up COM1 for 1200 baud, 8-bit, 1 stop, no parity */
   outp(COM_LCR(port), 0x80);
   outp(COM_DIVLO(port), 0x60);
   outp(COM_DIVHI(port), 0);
   outp(COM_LCR(port), 0x03);
   outp(COM_IER(port), 0);
   outp(COM_MCR(port), 0x03);

/* main loop */
   for(;;)
   {
   /* wait for a byte or error */
      line_status = 0;
      while(!(line_status & 0x1F))
      {
         printf("%c\b", waitBar[waitLoop=(waitLoop+1)%4]);
         line_status = inp(COM_LSR(port));
      }

   /* check for error */
      if(line_status & 0x1E)
      {
         printf(" Error: line_status=0x%X\n", line_status);
         break;
      }

   /* handle mouse protocol */
      if(mouseType == MS)
         msMouse(inp(COM_RXBUF(port)));
      else
         pcMouse(inp(COM_RXBUF(port)));
   }
}


void msMouse(int byte)
{
static int inSync = 0;
static int index = 0;
static char mouseMsg[3];

/* get in sync */
   if(!inSync)
   {
      index = 0;
      if((byte & 0xC0) == 0xC0)
         inSync = 1;
      else
         return;
   }

/* check we haven't lost sync */
   if(!index && (byte & 0xC0) != 0xC0)
   {
      puts("Lost sync");
      inSync = 0;
      return;
   }

/* add byte to message */
   mouseMsg[index++] = (char)byte & 0x7F;

/* check for complete message and display it */
   if(index == 3)
   {
      index = 0;
      if(mouseMsg[0] & 0x20)
         putchar('L');
      else
         putchar(' ');
      if(mouseMsg[0] & 0x10)
         putchar('R');
      else
         putchar(' ');
      if(mouseMsg[1] & 0x20)
         mouseMsg[1] |= 0xC0;
      if(mouseMsg[2] & 0x20)
         mouseMsg[2] |= 0xC0;
      printf(" x=%d y=%d\n", mouseMsg[1], mouseMsg[2]);
   }
}

void pcMouse(int byte)
{
static int inSync = 0;
static int index = 0;
static char mouseMsg[5];

/* get in sync */
   if(!inSync)
   {
      index = 0;
      if((byte & 0xF0) == 0x80)
         inSync = 1;
      else
         return;
   }

/* check we haven't lost sync */
   if(!index && (byte & 0xF0) != 0x80)
   {
      puts("Lost sync");
      inSync = 0;
      return;
   }

/* add byte to message */
   mouseMsg[index++] = (char)byte;

/* check for complete message and display it */
   if(index == 5)
   {
      index = 0;
      if(mouseMsg[0] & 0x04)
         putchar(' ');
      else
         putchar('L');
      if(mouseMsg[0] & 0x02)
         putchar(' ');
      else
         putchar('M');
      if(mouseMsg[0] & 0x01)
         putchar(' ');
      else
         putchar('R');
      printf(" x=%d y=%d dx=%d dy=%d\n", mouseMsg[1], mouseMsg[2],
         mouseMsg[3], mouseMsg[4]);
   }
}

