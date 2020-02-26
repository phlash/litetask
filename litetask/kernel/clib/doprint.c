/*-------------------------------------------------------------------------
   DOPRINT.C - LiteTask Kernel C-Library text string formatting

   $Author:   Phlash  $
   $Date:   16 Sep 1995 17:19:10  $
   $Revision:   1.1  $

-------------------------------------------------------------------------*/

#include "litetask.h"
#include "doprint.h"

static int near doInt(chOutFunc_t pOut, void far *arg, int far *pi, int doZero)
{
int val = *pi;
int cnt = 0;

// Check sign
   if(val < 0)
   {
      pOut(arg, '-');
      cnt++;
      val = -val;
   }

// Generate up to a 5 digit number, starting with 10,000's
   if(val >= 10000)
   {
      pOut(arg, (val / 10000) + '0');
      cnt++;
      val = val % 10000;
      doZero++;
   }
   if(val >= 1000 || doZero)
   {
      pOut(arg, (val / 1000) + '0');
      cnt++;
      val = val % 1000;
      doZero++;
   }
   if(val >= 100 || doZero)
   {
      pOut(arg, (val / 100) + '0');
      cnt++;
      val = val % 100;
      doZero++;
   }
   if(val >= 10 || doZero)
   {
      pOut(arg, (val / 10) + '0');
      cnt++;
      val = val % 10;
      doZero++;
   }
   pOut(arg, val + '0');
   cnt++;

// Return number of characters generated
   return cnt;
}

static int near doLong(chOutFunc_t pOut, void far *arg, long far *pl)
{
long lval = *pl;
int cnt = 0;
int high, med, low, doZero = 0;

// Check sign
   if(lval < 0L)
   {
      pOut(arg, '-');
      cnt++;
      lval = -lval;
   }

// Divide into three integers, the 100M, 10,000s and the units
   high = lval / 100000000L;
   med  = (lval % 100000000L) / 10000;
   low  = lval % 10000L;

// Process integers to form output string
   if(high)
      cnt += doInt(pOut, arg, &high, doZero++);
   if(med)
      cnt += doInt(pOut, arg, &med, doZero++);
   cnt += doInt(pOut, arg, &low, doZero);
   return cnt;
}

static int near doHex(chOutFunc_t pOut, void far *arg, unsigned short far *px)
{
unsigned short val = *px, doZero = 0;
int cnt = 0;

// Generate up to a 4 digit hexadecimal number
   if(val >= 0x1000)
   {
      pOut(arg, (val / 0x1000 > 9) ? (val / 0x1000) - 10 + 'A' : (val / 0x1000) + '0');
      cnt++;
      val = val % 0x1000;
      doZero++;
   }
   if(val >= 0x100 || doZero)
   {
      pOut(arg, (val / 0x100 > 9) ? (val / 0x100) - 10 + 'A' : (val / 0x100) + '0');
      cnt++;
      val = val % 0x100;
      doZero++;
   }
   if(val >= 0x10 || doZero)
   {
      pOut(arg, (val / 0x10 > 9) ? (val / 0x10) - 10 + 'A' : (val / 0x10) + '0');
      cnt++;
      val = val % 0x10;
      doZero++;
   }
   pOut(arg, (val > 9) ? val - 10 + 'A' : val + '0');
   cnt++;

// Return number of characters generated
   return cnt;
}

static int near doString(chOutFunc_t pOut, void far *arg, char far * far *ps)
{
char far *s = *ps;
int cnt = 0;

// Copy supplied string into output
   while(*s)
      pOut(arg, *s++), cnt++;

// Return the number of characters copied
   return cnt;
}

int far doPrint(chOutFunc_t pOut, void far *arg, char far *fmt, char far *argp)
{
int cnt = 0;
char c;

/* sanity checks */
   if(pOut == NULL || fmt == NULL || argp == NULL)
      return 0;

/* process format string */
   while(*fmt)
   {
      switch(c = *fmt++)
      {
      case '%':
         switch(c = *fmt++)
         {
         case 'c':
            pOut(arg, *argp);
            cnt++;
            argp += sizeof(int);
            break;

         case 'i':
            cnt += doInt(pOut, arg, argp, 0);
            argp += sizeof(int);
            break;

         case 'x':
            cnt += doHex(pOut, arg, argp);
            argp += sizeof(short);
            break;

         case 'l':
            cnt += doLong(pOut, arg, argp);
            argp += sizeof(long);
            break;

         case 's':
            cnt += doString(pOut, arg, argp);
            argp += sizeof(char far *);
            break;

         default:
            pOut(arg, c);
            cnt++;
         }
         break;

      default:
         pOut(arg, c);
         cnt++;
      }
   }
   return cnt;
}

/* End */
