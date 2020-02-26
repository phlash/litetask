#include <stdio.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include "coff.h"

#define pause()   puts("Press <CR> to continue.."); \
                  getchar();

void dumpAout(int coffFile, int size)
{
AOUTHDR aOutHeader;
GNU_AOUT gnuAOut;

   if(size == AOUTSZ)
   {
      if(read(coffFile, (char *)&aOutHeader, sizeof(aOutHeader)) != sizeof(aOutHeader))
      {
         perror("reading COFF file");
         return;
      }
      printf("Unix a.out header:\n");
      printf("magic:\t0x%04X\n", aOutHeader.magic);
      printf("vstamp:\t0x%04X\n", aOutHeader.vstamp);
      printf("tsize:\t%li\n", aOutHeader.tsize);
      printf("dsize:\t%li\n", aOutHeader.dsize);
      printf("bsize:\t%li\n", aOutHeader.bsize);
      printf("entry:\t0x%08lX\n", aOutHeader.entry);
      printf("tstart:\t0x%08lX\n", aOutHeader.text_start);
      printf("dstart:\t0x%08lX\n", aOutHeader.data_start);
   }
   else if(size == sizeof(GNU_AOUT))
   {
      printf("GNU a.out header:\n");
      lseek(coffFile, (long)size, SEEK_CUR);
   }
   else
   {
      printf("Unknown optional header\n");
      lseek(coffFile, (long)size, SEEK_CUR);
   }
}

void dumpScn(int scn, int coffFile)
{
SCNHDR scnHeader;

   if(read(coffFile, (char *)&scnHeader, sizeof(scnHeader)) != sizeof(scnHeader))
   {
      perror("reading COFF file");
      return;
   }
   printf("Section: %i\n", scn);
   printf("\tname:\t%.8s\n", scnHeader.s_name);
   printf("\tpaddr:\t0x%08lX\n", scnHeader.s_paddr);
   printf("\tvaddr:\t0x%08lX\n", scnHeader.s_vaddr);
   printf("\tsize:\t%li\n", scnHeader.s_size);
   printf("\tscnptr:\t0x%08lX\n", scnHeader.s_scnptr);
   printf("\trelptr:\t0x%08lX\n", scnHeader.s_relptr);
   printf("\tlnoptr:\t0x%08lX\n", scnHeader.s_lnnoptr);
   printf("\tnreloc:\t%i\n", scnHeader.s_nreloc);
   printf("\tnlno:\t%i\n", scnHeader.s_nlnno);
   printf("\tflags:\t0x%08lX\n", scnHeader.s_flags);
}

void dumpSyms(int coffFile, long symOff, int nSyms)
{
int i, j;
SYMENT sym;
AUXENT aux;

   lseek(coffFile, symOff, SEEK_SET);
   for(i=0; i<nSyms; i++)
   {
      if(read(coffFile, (char *)&sym, sizeof(sym)) != sizeof(sym))
      {
         perror("reading symbol table");
         return;
      }
      printf("%.8s\tVal=0x%08lX\tScn=%d\tType=0x%04X\tClass=",
         sym.e.e_name,
         sym.e_value,
         sym.e_scnum,
         sym.e_type);
      switch(sym.e_sclass)
      {
      case C_EFCN:  /* end of function */
            printf("End of function\n");
            break;
      case C_NULL:  /* null symbol */
            printf("Null\n");
            break;
      case C_AUTO:  /* automatic variable */
            printf("Automatic\n");
            break;
      case C_EXT:  /* external symbol */
            printf("External symbol\n");
            break;
      case C_STAT:  /* static */
            printf("Static\n");
            break;
      case C_REG:  /* register variable    */
            printf("Register variable\n");
            break;
      case C_EXTDEF:  /* external definition     */
            printf("External def.\n");
            break;
      case C_LABEL:  /* label       */
            printf("Label\n");
            break;
      case C_ULABEL:  /* undefined label      */
            printf("Undefined label\n");
            break;
      case C_MOS:  /* member of structure     */
            printf("Member of structure\n");
            break;
      case C_ARG:  /* function argument    */
            printf("Function argument\n");
            break;
      case C_STRTAG: /* structure tag     */
            printf("Structure tag\n");
            break;
      case C_MOU: /* member of union      */
            printf("Member of union\n");
            break;
      case C_UNTAG: /* union tag         */
            printf("Union tag\n");
            break;
      case C_TPDEF: /* type definition      */
            printf("Type definition\n");
            break;
      case C_USTATIC: /* undefined static     */
            printf("Undefined static\n");
            break;
      case C_ENTAG: /* enumeration tag      */
            printf("Enumeration tag\n");
            break;
      case C_MOE: /* member of enumeration   */
            printf("Member of enumeration\n");
            break;
      case C_REGPARM: /* register parameter      */
            printf("Register parameter\n");
            break;
      case C_FIELD: /* bit field         */
            printf("Bitfield\n");
            break;
      case C_AUTOARG: /* auto argument     */
            printf("Auto argument\n");
            break;
      case C_LASTENT: /* dummy entry (end of block) */
            printf("Block end\n");
            break;
      case C_BLOCK:   /* ".bb" or ".eb"    */
            printf("Block start\n");
            break;
      case C_FCN:   /* ".bf" or ".ef"    */
            printf("Function\n");
            break;
      case C_EOS:   /* end of structure     */
            printf("End of structure\n");
            break;
      case C_FILE:   /* file name         */
            printf("File name\n");
            break;
      case C_LINE:   /* line # reformatted as symbol table entry */
            printf("Line number\n");
            break;
      case C_ALIAS:   /* duplicate tag     */
            printf("Alias\n");
            break;
      case C_HIDDEN:   /* ext symbol in dmert public lib */
            printf("Hidden\n");
            break;
      default:
            printf("Unknown\n");
            break;
      }
      for(j=0; j<sym.e_numaux; j++, i++)
      {
         if(read(coffFile, (char *)&aux, sizeof(aux)) != sizeof(aux))
         {
            perror("reading symbol table");
            return;
         }
         if(sym.e_sclass == C_FILE)
            printf("\tFile name: %.14s\n", aux.x_file.x_fname);
         else
            printf("\tSymbol: line %d size %d\n",
               aux.x_sym.x_misc.x_lnsz.x_lnno,
               aux.x_sym.x_misc.x_lnsz.x_size);

#ifdef never
union external_auxent {
   struct {
      unsigned long x_tagndx;    /* str, un, or enum tag indx */
      union {
         struct {
             unsigned short  x_lnno;            /* declaration line number */
             unsigned short  x_size;            /* str/union/array size */
         } x_lnsz;
         unsigned long x_fsize;     /* size of function */
      } x_misc;
      union {
         struct {             /* if ISFCN, tag, or .bb */
             unsigned long x_lnnoptr;  /* ptr to fcn line # */
             unsigned long x_endndx;   /* entry ndx past block end */
         } x_fcn;
         struct {             /* if ISARY, up to 4 dimen. */
             unsigned short x_dimen[E_DIMNUM];
         } x_ary;
      } x_fcnary;
      unsigned short x_tvndx;                /* tv index */
   } x_sym;

   union {
      char x_fname[E_FILNMLEN];
      struct {
         unsigned long x_zeroes;
         unsigned long x_offset;
      } x_n;
   } x_file;

   struct {
      unsigned long x_scnlen;    /* section length */
      unsigned short x_nreloc;               /* # relocation entries */
      unsigned short x_nlinno;               /* # line numbers */
   } x_scn;

        struct {
      unsigned long x_tvfill;    /* tv fill value */
      unsigned short x_tvlen;                /* length of .tv */
      unsigned short x_tvran[2];             /* tv range */
   } x_tv;     /* info about .tv section (in auxent of symbol .tv)) */


};
#endif

      }
   }
}

#define DO_HDR 1
#define DO_SCN 2
#define DO_SYM 4

void main(int argc, char **argv)
{
int coffFile, scn, bits;
FILHDR fileHeader;
time_t cofft;

   if(argc < 2)
   {
      puts("usage: fileinfo <COFF file>");
      return;
   }
   if(argc > 2)
   {
      bits = 0;
      for(scn=2; scn<argc; scn++)
      {
         switch(argv[scn][0])
         {
         case 'h':               /* Header */
         case 'H':
            bits |= DO_HDR;
            break;
         case 't':               /* secTions */
         case 'T':
            bits |= DO_SCN;
            break;
         case 's':               /* Symbols */
         case 'S':
            bits |= DO_SYM;
            break;
         default:
            printf("Invalid argument %s ignored\n", argv[scn]);
         }
      }
   }
   else
      bits = DO_HDR | DO_SCN | DO_SYM;
   if((coffFile = open(argv[1], O_RDONLY|O_BINARY)) == -1)
   {
      perror("opening COFF file");
      return;
   }
   if(read(coffFile, (char *)&fileHeader, sizeof(fileHeader)) != sizeof(fileHeader))
   {
      perror("reading COFF file");
      return;
   }
   printf("File: %s\n", argv[1]);
   if(bits & DO_HDR)
   {
      printf("magic:\t0x%04X\n", fileHeader.f_magic);
      printf("scns:\t%i\n", fileHeader.f_nscns);
      if(fileHeader.f_timdat)
      {
         cofft = fileHeader.f_timdat;
         printf("date:\t%s", ctime(&cofft));
      }
      else
         printf("date:\t(unspecified)\n");
      printf("symptr:\t0x%08lX\n", fileHeader.f_symptr);
      printf("nsyms:\t%li\n", fileHeader.f_nsyms);
      printf("opthdr:\t%i\n", fileHeader.f_opthdr);
      printf("flags:\t0x%04X\n\n", fileHeader.f_flags);

      if(fileHeader.f_opthdr)
         dumpAout(coffFile, fileHeader.f_opthdr);
      pause();
   }
   if(bits & DO_SCN)
      for(scn=0; scn<fileHeader.f_nscns; scn++)
      {
         dumpScn(scn, coffFile);
         pause();
      }

   if(bits & DO_SYM)
      dumpSyms(coffFile, fileHeader.f_symptr, fileHeader.f_nsyms);
   close(coffFile);
}
