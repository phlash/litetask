/*------------------------------------------------------------------------
   STRING.C - LiteTask string manipulation routines

   $Author:   Phlash  $
   $Date:   05 Feb 1995 21:49:20  $
   $Revision:   1.1  $

------------------------------------------------------------------------*/

#include "litetask.h"

char far * far strcpy(char far *s1, char far *s2)
{
int i;

   for(i=0; s2[i]; i++)
      s1[i] = s2[i];
   s1[i] = 0;
   return s1;
}

char far * far strcat(char far *s1, char far *s2)
{
int i,j;

   for(i=0; s1[i]; i++)
      ;
   for(j=0; s2[j]; j++)
      s1[i+j] = s2[j];
   s1[i+j] = 0;
   return s1;
}

int far strlen(char far *s)
{
int i;

   for(i=0; s[i]; i++)
      ;
   return i;
}

int far strcmp(char far *s1, char far *s2)
{
int i;

   for(i=0; s1[i] || s2[i]; i++)
      if(s1[i] != s2[i])
         return (s1[i] > s2[i]) ? 1 : -1;
   return 0;
}

char far * far strchr(char far *s, char c)
{
int i;

   for(i=0; s[i]; i++)
      if(s[i] == c)
         return &s[i];
   return NULL;
}

char far * far strrchr(char far *s, char c)
{
int i;

   for(i=strlen(s)-1; i>=0; i--)
      if(s[i] == c)
         return &s[i];
   return NULL;
}

char far * far strstr(char far *s1, char far *s2)
{
int i,j;

   for(i=0; s1[i]; i++)
   {
      for(j=0; s1[i+j] && s2[j] && s1[i+j]==s2[j]; j++)
         ;
      if(!s2[j])
         return &s1[i];
   }
   return NULL;
}

/* End */
