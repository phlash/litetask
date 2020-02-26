#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include "dllload.h"

void main(int argc, char **argv)
{
int exeFile, load, i;
exeHeader_t fileHeader;
WORD programParagraphs;
WORD relocation;
WORD far *relocationEntry;
BYTE far *programSpace;
WORD (far *entryPoint)(symbol_t far * far *psymTbl);
WORD nEntries, entry;
symbol_t far *symTbl;

/* open the file */
   if(argc < 2)
   {
      puts("usage: exedisp <.EXE file name> [-load]");
      return;
   }
   if(argc > 2)
      load = 1;
   else
      load = 0;

   exeFile = open(argv[1], O_RDONLY | O_BINARY);
   if(exeFile < 0)
   {
      fprintf(stderr, "Cannot open file %s\n", argv[1]);
      return;
   }

/* read the .EXE header */
   if(read(exeFile, (char *)&fileHeader, sizeof(fileHeader)) != sizeof(fileHeader))
   {
      fprintf(stderr, "Invalid file size\n");
      return;
   }

/* check the sig */
   if(strncmp(fileHeader.exeSig, "MZ", 2))
   {
      fprintf(stderr, "Invalid file format\n");
      return;
   }

/* now display the data */
   printf("file: %s\n", argv[1]);
   printf("lastBlockSize:       %u\n", fileHeader.lastBlockSize);
   printf("fileBlocks:          %u\n", fileHeader.fileBlocks);
   printf("relocations:         %u\n", fileHeader.relocations);
   printf("headerParagraphs:    %u\n", fileHeader.headerParagraphs);
   printf("minAllocation:       %u\n", fileHeader.minAllocation);
   printf("maxAllocation:       %u\n", fileHeader.maxAllocation);
   printf("offsetToSS:          %XH\n", fileHeader.offsetToSS);
   printf("initialSP:           %XH\n", fileHeader.initialSP);
   printf("checksum:            %u\n", fileHeader.checksum);
   printf("initialIP:           %XH\n", fileHeader.initialIP);
   printf("offsetToCS:          %XH\n", fileHeader.offsetToCS);
   printf("offsetToRelocations: %u\n", fileHeader.offsetToRelocations);
   printf("overlayNumber:       %u\n", fileHeader.overlayNumber);

/* allocate memory for program */
   programParagraphs = (fileHeader.fileBlocks * 32) -
                       (fileHeader.headerParagraphs) +
                       (fileHeader.minAllocation);
   printf("Program requires %u paragraphs of memory\n", programParagraphs);
   if(_dos_allocmem(programParagraphs, &(FP_SEG(programSpace))))
   {
      fprintf(stderr, "Insufficient memory to load program\n");
      return;
   }
   FP_OFF(programSpace) = 0;

/* load the program into memory */
   lseek(exeFile, (long)fileHeader.headerParagraphs * 16L, SEEK_SET);
   if(read(exeFile, programSpace, programParagraphs * 16) < 0)
   {
      fprintf(stderr, "Cannot read .EXE file\n");
      return;
   }

/* display the relocation table and relocate the loaded program */
   lseek(exeFile, (long)fileHeader.offsetToRelocations, SEEK_SET);
   printf("Relocations at:\n");
   for(relocation = 0; relocation < fileHeader.relocations; relocation++)
   {
      read(exeFile, (char *)&relocationEntry, sizeof(relocationEntry));
      printf("%lp\n", relocationEntry);
      FP_SEG(relocationEntry) += FP_SEG(programSpace);
      *relocationEntry += FP_SEG(programSpace);
   }

/* close the load file */
   close(exeFile);

/* see if we're done */
   if(!load)
   {
      _dos_freemem(FP_SEG(programSpace));
      return;
   }

/* now set registers and call DLL */
   FP_SEG(entryPoint) = fileHeader.offsetToCS + FP_SEG(programSpace);
   FP_OFF(entryPoint) = fileHeader.initialIP;
   nEntries = (*entryPoint)(&symTbl);
   printf("DLL entry point returned: %u entries @ %lp\n", nEntries, symTbl);

/* display symbol table */
   for(entry=0; entry<nEntries; entry++)
   {
      switch(symTbl[entry].flags)
      {
      case CODE_IMPORT:
         symTbl[entry].ptr = puts;
         printf("Unresolved function: ");
         break;
      case CODE_EXPORT:
         (*symTbl[entry].ptr)();
         printf("Exported @ %lp function: ", symTbl[entry].ptr);
         break;
      case DATA_IMPORT:
         printf("Unresolved data: ");
         break;
      case DATA_EXPORT:
         printf("Exported @ %lp data: ", symTbl[entry].ptr);
         break;
      default:
         printf("Invalid symbol table entry\n");
         continue;
      }
      for(i=0; symTbl[entry].name[i]; i++)
         fputc(symTbl[entry].name[i], stdout);
      fputc('\n', stdout);
   }

/* free allocated memory and exit */
   _dos_freemem(FP_SEG(programSpace));
}

