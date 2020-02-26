/*------------------------------------------------------------------------
   GETOPTS.C - LiteTask options parser

   $Author:   Phlash  $
   $Date:   01 Apr 1995 14:43:38  $
   $Revision:   1.1  $

   NOTES:
   1) The options string is a list of allowed characters, with a colon
      after any which may require an argument eg: "ab:c".
------------------------------------------------------------------------*/
#ifdef TRY_OPTS
#include <stdio.h>
#else
#include "litetask.h"
#endif

static int cmdIdx=0;               /* where we are in the string */
static int inOpts=0;               /* are we parsing opts? */

char far getopts(char far *cmd, char far *opts, char far * far *pOptArg)
{
int optIdx;

/* sanity check */
   if(!cmd || !opts)
      return (char)-1;

/* parse the cmd buffer for the next option */
   while(cmd[cmdIdx])
   {
      if(inOpts)                                /* if parsing options.. */
      {
         for(optIdx=0; opts[optIdx]; optIdx++)  /* try each in turn */
         {
            if(':' == opts[optIdx])             /* skip colon fields */
               continue;
            if(cmd[cmdIdx] == opts[optIdx])     /* match option? */
            {
               cmdIdx++;
               if(':' == opts[optIdx+1])        /* need an optArg? */
               {
                  while(cmd[cmdIdx] && ' ' == cmd[cmdIdx])
                     cmdIdx++;
                  if(cmd[cmdIdx])               /* valid arg, not in opts */
                  {
                     inOpts = 0;
                     *pOptArg = &cmd[cmdIdx];
                     while(cmd[cmdIdx] && cmd[cmdIdx] != ' ')
                        cmdIdx++;
                  }
                  else                          /* invalid arg (EOL) */
                     *pOptArg = NULL;
               }
               return opts[optIdx];
            }
         }
         inOpts = 0;
      }
      if('-' == cmd[cmdIdx])                    /* check for another option */
         inOpts = 1;
      cmdIdx++;                                 /* keep looking.. */
   }

/* end of the cmd buffer, return 0 */
   return (char)0;
}

void far resetopts(void)
{
   cmdIdx = 0;
   inOpts = 0;
}

#ifdef TRY_OPTS
void main(int argc, char **argv)
{
int i, done=0;
char far *opArg, cmd[128], far *opts = "abc:d-";

   if(argc < 2)
   {
      puts("usage: getopts -[a|b|d] [-c <arg>] [--] <other stuff>");
      return;
   }
   cmd[0] = 0;
   for(i=1; i<argc; i++)
   {
      strcat(cmd, argv[i]);
      strcat(cmd, " ");
   }
   printf("Parsing options from: %s\n", cmd);
   while(!done)
   {
      switch(getopts(cmd, opts, &opArg))
      {
      case -1:
         fprintf(stderr, "Error parsing options!\n");
         return;
      case 0:
         done = 1;
         break;
      case 'a':
         puts("a=true");
         break;
      case 'b':
         puts("b=true");
         break;
      case 'c':
         printf("c=");
         if(opArg)
            for(i=0; opArg[i] && opArg[i] != ' '; i++)
               putchar(opArg[i]);
         else
            printf("(no arg)");
         printf("\n");
         break;
      case 'd':
         puts("d=true");
         break;
      case '-':
         puts("-=true");
         break;
      default:
         fprintf(stderr, "Invalid return value from getopts()\n");
         return;
      }
   }
}
#endif
