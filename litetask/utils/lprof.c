/*------------------------------------------------------------------------
   LPROF.C - LiteTask profile dump analyser

   $Author:$
   $Date:$
   $Revision:$

------------------------------------------------------------------------*/
#include <stdio.h>
#include <io.h>
#include <malloc.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

typedef unsigned short WORD;
typedef unsigned long DWORD;

typedef struct {
            WORD *buf;
            WORD len, lck, tsw, isw, swt;
            } pbuf_t;

typedef struct {
            char sym[34];
            WORD off, ticks;
            } sym_t;

#define MAXSYMS 512
sym_t symTable[MAXSYMS];
int nSyms = 0;

pbuf_t readProfFile(char *profFile)
{
int pf;
WORD pbl;
pbuf_t pbuf;

   pbuf.buf = NULL;
   pbuf.len = 0;
   if((pf = open(profFile, O_RDONLY | O_BINARY)) < 0)
   {
      perror("opening profile data file");
      return pbuf;
   }
   pbl = (WORD)filelength(pf) / sizeof(WORD) - 4;
   pbuf.buf = (WORD *)malloc(pbl * sizeof(WORD));
   if(!pbuf.buf)
   {
      perror("allocating profile buffer");
      return pbuf;
   }
   if( (read(pf, (char *)&pbuf.lck, sizeof(WORD)) == -1) ||
       (read(pf, (char *)&pbuf.tsw, sizeof(WORD)) == -1) ||
       (read(pf, (char *)&pbuf.isw, sizeof(WORD)) == -1) ||
       (read(pf, (char *)&pbuf.swt, sizeof(WORD)) == -1) ||
       (read(pf, (char *)pbuf.buf, pbl * sizeof(WORD)) == -1))
   {
      perror("reading profile data file");
      free(pbuf.buf);
      return pbuf;
   }
   close(pf);
   pbuf.len = pbl;
   printf("Read %u bytes of profile data\n", pbuf.len * sizeof(WORD));
   return pbuf;
}

int readMapFile(char *mapFile)
{
FILE *mf;
char line[80], symbol[34];
WORD seg, off, poff;

   mf = fopen(mapFile, "r");
   if(!mf)
   {
      perror("opening map file");
      return -1;
   }
   while(!feof(mf))
   {
      fgets(line, sizeof(line), mf);
      if(strstr(line, "Publics by Value"))
         break;
   }
   if(feof(mf))
   {
      fprintf(stderr, "EOF reading map file %s\n", mapFile);
      return -1;
   }
   fscanf(mf, "\n");
   while(!feof(mf))
   {
      if(fscanf(mf, "%x:%x %32s\n", &seg, &off, symbol) != 3)
         break;
      poff = (seg << 4) + off;
      strncpy(symTable[nSyms].sym, symbol, sizeof(symTable[nSyms].sym));
      symTable[nSyms].off = poff;
      symTable[nSyms].ticks = 0;
      nSyms++;
      if(nSyms >= MAXSYMS)
      {
         fprintf(stderr, "Maximum symbol table size reached\n");
         break;
      }
   }
   fclose(mf);
   printf("Read %d symbols\n", nSyms);
   return 0;
}

void main(int argc, char **argv)
{
WORD pi;
DWORD tot = 0L;
pbuf_t pbuf;
int sym;

   if(argc < 3)
   {
      puts("usage: lprof <program map file> <profile data file>");
      return;
   }
   if(readMapFile(argv[1]))
      return;
   pbuf = readProfFile(argv[2]);
   if(!pbuf.len)
      return;
   puts("Press <CR>");
   getchar();
   printf("Scheduler lock-outs: %u\n", pbuf.lck);
   printf("Scheduler task-switches: %u\n", pbuf.tsw);
   printf("Scheduler idle-switches: %u\n", pbuf.isw);
   printf("Average scheduler switching time: %u usecs\n", pbuf.swt);
   for(pi = 0; pi < pbuf.len; pi++)
   {
      if(pbuf.buf[pi])
      {
         tot += (DWORD)pbuf.buf[pi];
         for(sym = 0; sym < nSyms; sym++)
         {
            if(symTable[sym].off == pi)
               break;
            if(symTable[sym].off > pi)
            {
               sym--;
               break;
            }
         }
         if(sym >= nSyms)
            sym--;
         symTable[sym].ticks += pbuf.buf[pi];
      }
   }
   printf("Profiler ran for %lu ticks. ", tot);
   if(tot < (DWORD)pbuf.len)
      puts("This may not be sufficient to acheive a good analysis");
   else
      putchar('\n');
   for(sym = 0; sym < nSyms; sym++)
   {
      if(symTable[sym].ticks)
         printf("%d\tticks in %s\n", symTable[sym].ticks, symTable[sym].sym);
   }
}
