#include "litetask.h"
#include "debug.h"

int mainStackSize = MINSTACK;

void far mainTask(char far *commandLine)
{
char far *str = "\"Hello World\"";
char buf[100];

/* Test printk's abilities */
   biosStr("biosStr() with %% and %' in it.\r\n");
   printk("printk() with %% and %' in it.\r\n");
   printk("printk():- Char: %c Int: %i Long: %l Hex: %x String: %s\r\n",
      'X', -5, 987654321L, 0xF00D, str);
   delayTask(18L);
   sprintf(buf,
      "sprintf():- Char: %c Int: %i Long: %l Hex: %x String: %s\r\n",
      'X', 5, -987654321L, 0xB00B, str);
   biosStr(buf);
}

/* End */
