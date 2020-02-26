#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <dos.h>
#include "coff.h"
#include "dtable.h"
#include "goprot.h"

/* Aggregate of expected COFF headers */
typedef struct {
         FILHDR fHdr;
         AOUTHDR aOut;
         SCNHDR textScn, dataScn, bssScn;
         } coffHeader_t;

void biosStr(char *str)
{
union REGS regs;

   while(*str)
   {
      regs.h.ah = 0x0E;
      regs.h.al = *str++;
      regs.x.bx = 7;
      int86(0x10, &regs, &regs);
   }
}

int biosKey(void)
{
union REGS regs;

   regs.x.ax = 0;
   int86(0x16, &regs, &regs);
   return regs.x.ax;
}

int loadSection(int cf, SCNHDR *pScn, unsigned long bAddr)
{
unsigned long i;
int n;
char buf[512], far *lp;

   switch((int)pScn->s_flags)
   {
   case STYP_TEXT:
      printf("Loading .text section @ 0x%lX ", pScn->s_paddr + bAddr);
      break;
   case STYP_DATA:
      printf("Loading .data section @ 0x%lX ", pScn->s_paddr + bAddr);
      break;
   case STYP_BSS:
      printf("Zeroing .bss section @ 0x%lX ", pScn->s_paddr + bAddr);
      for(i=0; i<pScn->s_size; i++)
      {
         FP_SEG(lp) = (unsigned short)((bAddr + pScn->s_paddr + i) >> 4);
         FP_OFF(lp) = (unsigned short)((bAddr + pScn->s_paddr + i) & 0x0F);
         *lp = 0;
      }
      return 0;
   default:
      fprintf(stderr, "Unknown section type 0x%08lX\n", pScn->s_flags);
      return -1;
   }
   lseek(cf, pScn->s_scnptr, SEEK_SET);
   for(i=0; i<pScn->s_size; i+=512L)
   {
      if((pScn->s_size - i) < 512L)
         n = (int)(pScn->s_size - i);
      else
         n = 512;
      if (read(cf, buf, n) != n)
      {
         perror("reading COFF section");
         return -1;
      }
      FP_SEG(lp) = (unsigned short)((bAddr + pScn->s_paddr + i) >> 4);
      FP_OFF(lp) = (unsigned short)((bAddr + pScn->s_paddr + i) & 0x0F);
      while(n--)
         lp[n] = buf[n];
      putchar('.');
   }
   puts("*");
   return 0;
}

void moveCoff(coffHeader_t *pcoff, unsigned long bAddr)
{
char far *src, far *dst;
unsigned long len, i;

/* Copy COFF program down to correct location in memory */
   printf("Moving COFF program to run-time address: 0x%lX\n",
      pcoff->textScn.s_paddr);
   len = pcoff->bssScn.s_paddr +
         pcoff->bssScn.s_size -
         pcoff->textScn.s_paddr;
   for(i=0; i<len; i++)
   {
      FP_SEG(src) = (unsigned short)((bAddr + pcoff->textScn.s_paddr + i) >> 4);
      FP_OFF(src) = (unsigned short)((bAddr + pcoff->textScn.s_paddr + i) & 0xF);
      FP_SEG(dst) = (unsigned short)((i + pcoff->textScn.s_paddr) >> 4);
      FP_OFF(dst) = (unsigned short)((i + pcoff->textScn.s_paddr) & 0xF);
      *dst = *src;
   }
}

void runCoff(unsigned long entry)
{
descriptor_t gdt[3];

   memset(&gdt[0], 0, sizeof(gdt[0]));
   codeSegment(&gdt[1], 0L, 0xFFFFFL);
   dataSegment(&gdt[2], 0L, 0xFFFFFL);
   loadGdt(sizeof(gdt)-1, gdt);
   biosStr("Press a key to execute COFF program:\r\n");
   biosKey();
   goProt(8, entry, 16, 16, 0xA0000);
}

void reLoadMe(int argc, char **argv, unsigned long bAddr)
{
unsigned int size;
char lb[20];
char *largv[80];
int arg;

/* Read free DOS memory */
   _dos_allocmem(0xFFFF, &size);
   _doserrno = 0;
   errno = 0;

/* Now allocate all but 64k */
   size -= 4096;
   if(_dos_allocmem(size, &size))
   {
      perror("allocating reload buffer");
      return;
   }

/* Re-spawn myself, adding '-loadbase address' as last argument */
   for(arg=0; arg<argc; arg++)
      largv[arg] = argv[arg];
   sprintf(lb, "%lu", bAddr);
   largv[arg++] = "-loadbase";
   largv[arg++] = lb;
   largv[arg++] = NULL;
   if(spawnv(P_WAIT, largv[0], largv))
   {
      perror("re-loading myself");
      return;
   }
   puts("Returned from reload!");
}

void main(int argc, char **argv)
{
int cf;
void far *fp;
unsigned long bAddr;
coffHeader_t coffHdr;

   if(argc < 2)
   {
      puts("usage: loadcoff <COFF file>");
      return;
   }
   if(strcmp(argv[argc-2],"-loadbase"))
   {
      fp = (void far *)main;
      bAddr = (unsigned long)(FP_SEG(fp)) << 4;
      printf("bAddr is: %lX\n", bAddr);
      reLoadMe(argc, argv, bAddr);
      return;
   }
   sscanf(argv[argc-1], "%lu", &bAddr);
   printf("bAddr is: %lX\n", bAddr);
   if((cf = open(argv[1], O_RDONLY|O_BINARY)) < 0)
   {
      perror("opening COFF file");
      return;
   }
   if(read(cf, (char *)&coffHdr, sizeof(coffHdr)) != sizeof(coffHdr))
   {
      perror("reading COFF file headers");
      close(cf);
      return;
   }
   if(!(coffHdr.fHdr.f_flags & F_EXEC))
   {
      fprintf(stderr, "COFF file is not executable\n");
      close(cf);
      return;
   }
   if(coffHdr.fHdr.f_nscns != 3)
   {
      fprintf(stderr, "COFF file does not have 3 sections\n");
      close(cf);
      return;
   }
   if(coffHdr.fHdr.f_opthdr != sizeof(AOUTHDR))
   {
      fprintf(stderr, "COFF file does not have a.out header\n");
      close(cf);
      return;
   }
   if(loadSection(cf, &coffHdr.textScn, bAddr) || 
      loadSection(cf, &coffHdr.dataScn, bAddr) || 
      loadSection(cf, &coffHdr.bssScn, bAddr))
   {
      close(cf);
      return;
   }
   close(cf);
   moveCoff(&coffHdr, bAddr);
   runCoff(coffHdr.aOut.entry);
}
