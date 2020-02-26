/*------------------------------------------------------------------------
   CLIBTEST.C - LiteTask Kernel C Library Tests

   $Author:$
   $Date:$
   $Revision:$

------------------------------------------------------------------------*/
#include "litetask.h"

/* BIOS I/O tests */

void BIOSKeyFlags(int flags, int extended)
{
/* Standard keyboard flags */
   if(flags & BKF_RSHIFT)
      biosStr("RS ");
   else
      biosStr("rs ");
   if(flags & BKF_LSHIFT)
      biosStr("LS ");
   else
      biosStr("ls ");
   if(flags & BKF_CTRL)
      biosStr("CTL ");
   else
      biosStr("ctl ");
   if(flags & BKF_ALT)
      biosStr("ALT ");
   else
      biosStr("alt ");
   if(flags & BKF_SCROLL)
      biosStr("SL ");
   else
      biosStr("sl ");
   if(flags & BKF_NUM)
      biosStr("NUM ");
   else
      biosStr("num ");
   if(flags & BKF_CAPS)
      biosStr("CAP ");
   else
      biosStr("cap ");
   if(flags & BKF_INSERT)
      biosStr("INS ");
   else
      biosStr("ins ");

   if(extended)
   {
   /* Extended keyboard flags */
      if(flags & BKF_LCTRL)
         biosStr("LCTL ");
      else
         biosStr("lctl ");
      if(flags & BKF_LALT)
         biosStr("LALT ");
      else
         biosStr("lalt ");
      if(flags & BKF_RCTRL)
         biosStr("RCTL ");
      else
         biosStr("rctl ");
      if(flags & BKF_RALT)
         biosStr("RALT ");
      else
         biosStr("ralt ");
      if(flags & BKF_SCROLLK)
         biosStr("SK ");
      else
         biosStr("sk ");
      if(flags & BKF_NUMK)
         biosStr("NK ");
      else
         biosStr("nk ");
      if(flags & BKF_CAPSK)
         biosStr("CK ");
      else
         biosStr("ck ");
      if(flags & BKF_SYSREQ)
         biosStr("SYS ");
      else
         biosStr("sys ");
   }
}

int BIOSTests(void)
{
char *str = "biosCh() test\r\n";
int i, flag;

   for(i=0; str[i]; i++)
      biosCh(str[i]);
   biosStr("biosStr() test\r\n");
   biosStr("biosKey(BK_PEEK/READ) test: Press a key..");
   while(!biosKey(BK_PEEK))
      ;
   biosCh(biosKey(BK_READ));
   biosStr("\r\n");
   biosStr("biosKey(BK_EPEEK/EREAD) test: Press an extended key..");
   while(!biosKey(BK_EPEEK))
      ;
   biosCh(biosKey(BK_EREAD));
   biosStr("\r\n");
   biosStr("biosKey(BK_FLAGS) test: Press some Ctrl-keys, <CR> to finish\r\n");
   while(!biosKey(BK_PEEK))
   {
      flag = biosKey(BK_FLAGS);
      BIOSKeyFlags(flag, 0);
      biosCh('\r');
   }
   biosKey(BK_READ);
   biosStr("\nbiosKey(BK_EFLAGS) test: Press some extened Ctrl-keys, then <CR>\r\n");
   while(!biosKey(BK_EPEEK))
   {
      flag = biosKey(BK_EFLAGS);
      BIOSKeyFlags(flag, 1);
      biosCh('\r');
   }
   biosKey(BK_EREAD);
   biosStr("\nBIOS Tests complete\r\n");
   return 0;
}

/* Port I/O tests */
int portIOTests(void)
{
int secs;

/* outp / outpw */
   biosStr("inp()/outp() test: should beep until a key is pressed..");
   while(!biosKey(BK_PEEK))
   {
      outp(0x61, inp(0x61) | 3);
      outp(0x61, inp(0x61) & 0xFC);
   }
   biosKey(BK_READ);
   biosStr("\r\ninpw() test: should display CMOS counter..\r\n");
   while(!biosKey(BK_PEEK))
   {
      secs=inpw(0x70);
      biosCh( (char)(secs/10) + '0' );
      biosCh( (char)(secs%10) + '0' );
      biosCh('\r');
   }
   biosKey(BK_READ);
   biosStr("\nport I/O tests complete\r\n");
   return 0;
}

/* sprintf test for later */
#ifdef SPRINTF
void main(void)
{
int i=5, x=0xFE10;
long j=87654321L;
char buffer[80], far *s="Testing";

   sprintf(buffer, "Char: %c Ints: %i %l Hex: 0x%x String: %s",
      'X', i, j, x, (char far *)s);
   puts(buffer);
}
#endif

void main(void)
{
   if(BIOSTests())
      return;
   if(portIOTests())
      return;
}

