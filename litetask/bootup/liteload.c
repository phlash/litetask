/* LITELOAD.C - Secondary loader program for 16/32-bit LiteTask Kernel(s)

   $Author:   Phlash  $
   $Date:   28 Jun 1995 21:40:56  $
   $Revision:   1.3  $

   Purpose - To locate and load the kernel file from a boot disk.
*/

#include "dos_disk.h"
#include "exe.h"
#include "coff.h"
#include "loadutil.h"
#include "dtable.h"
#include "goprot.h"

/* =========================== Macros =========================== */

/* Kernel types */
#define KERNEL_16    0
#define KERNEL_32    1

/* Kernel load address */
#define KERNEL_SEG   0x80

/* 32-bit stack address */
#define STACK_32     0xA0000

/* Far pointer manipulation */
#define NULL 0L
#define FP_SEG(ptr)  (*((unsigned short *)&ptr + 1))
#define FP_OFF(ptr)  (*((unsigned short *)&ptr))

/* Debugging output */
#define DEB(l, x)    if(debug >= l) bputs(x)
#define DEB1(x)      DEB(1, x)
#define DEB2(x)      DEB(2, x)

/* ========================== Internal types =========================== */
typedef struct {
      FILHDR  coffHdr;
      AOUTHDR aoutHdr;
      SCNHDR  textScn, dataScn, bssScn;
      } coffHeader_t;

/* ======================= Internal prototypes ========================= */
WORD findKernel(char far *name);
int identKernel(WORD cluster);
int loadExe(WORD cluster, WORD loadSeg);
int loadCoff(WORD cluster, WORD loadSeg);
int startExe(WORD loadSeg);
int startCoff(WORD loadSeg);
int read(WORD *pc, WORD *po, char far *addr, WORD len);
WORD nextFATCluster(WORD cluster);
int bputs(char *string);
char *bwtoh(WORD w);
int bcmp(char far *s1, char far *s2, int bytes);

/* =========================== Global data =========================== */
BYTE bootDrive = 0;                /* The current boot disk */
union diskSector far *boot = NULL; /* The boot sector */
int kernelType = -1;               /* The kernel type (16/32 bit) */

BYTE FATType = FAT12;              /* FAT type (12 or 16 bit) */
BYTE FATBuf[1024] = { 0 };         /* FAT table buffer (2 sectors) */
DWORD FATSector = 0xFFFFFFFFL;     /* Currently loaded FAT sector */

union diskSector diskBuf = { 0 };  /* disk sector buffer */
union diskSector bootBuf = { 0 };  /* boot sector buffer */
DWORD FATTableStart = 0L;          /* Start of FAT table (logical sector) */
DWORD rootDirStart  = 0L;          /* Start of root directory (log sector) */
DWORD fileAreaStart = 0L;          /* Start of files area (logical sector) */
int debug = 0;                     /* Debugging flag (see DEB() macro) */

exeHeader_t exeHdr;                /* .EXE file header buffer */
coffHeader_t coffHdr;              /* COFF file header buffer */

/* Version control string */
char pvcsId[] = "$Revision:   1.3  $\r\n";

/* =========================== Main program =========================== */
int main(BYTE driveID, WORD segVal)
{
WORD kernelCluster, kernelSeg;
DWORD totalSectors;
char far *cmdLine, far *kernelName = NULL;
int i, cmdlen;

/* Say Hi! */
   bputs("LiteLoad - 16/32-bit Kernel loader copyright (AshbySoft *) 1995\r\n");
   bputs("---------------------------------------------------------------\r\n");
   bputs(pvcsId);

/* see if we were loaded from a DOS prompt */
   if(driveID == 0xFF)
   {
   /* If so, parse command line for drive to search, debug flag, etc */
      FP_SEG(cmdLine) = segVal;
      FP_OFF(cmdLine) = 0x80;
      cmdlen = cmdLine[0];
      if(cmdlen > 0)
      {
         for(i=1; i<=cmdlen; i++)
         {
            switch(cmdLine[i])
            {
            case ' ':
            case '\t':
               break;
            case 'A':
            case 'a':
               bootDrive = 0;
               break;
            case 'B':
            case 'b':
               bootDrive = 1;
               break;
            case '1':
            case '2':
               debug = cmdLine[i] - '0';
               break;
            case 'f':
            case 'F':
               while(++i <= cmdlen)
               {
                  if(cmdLine[i] > ' ')
                  {
                     kernelName = &cmdLine[i];
                     break;
                  }
               }
               i += 11;
               break;
            default:
               bputs("Usage: liteload [a|b|c|1|2|f <file>]\r\n");
               bputs("where  a,b selects boot disk\r\n");
               bputs("       1,2 enables debugging at that level\r\n");
               bputs("       f <file> selects alternative kernel file\r\n");
               return 1;
            }
         }
      }
      else
      {
         bootDrive = 0;
      }
      bputs("LiteLoad - Reading boot sector from disk: ");
      if(bootDrive & 0x80)
         biosChar(bootDrive-0x80+'C');
      else
         biosChar(bootDrive + 'A');
      bputs("\r\n");
      boot = &bootBuf;
      if(biosRead(bootDrive, 1L, 1, NULL, boot->raw))
      {
         bputs("Cannot read boot disk, exiting..");
         return 1;
      }
      if(!kernelName)
         kernelName = boot->bpb.volumeLabel;

   /* Load kernel 64Kbytes above this program */
      kernelSeg = segVal + 0x1000;
   }
   else
   {
   /* enable debugging if either shift key is pressed, extra if both ! */
      switch(biosKey(2) & 0x3)
      {
      case 0:
         break;
      case 1:
      case 2:
         debug = 1;
         break;
      case 3:
         debug = 2;
         break;
      }

   /* Boot sector loaded us, so use it's data */
      FP_SEG(boot) = segVal;
      FP_OFF(boot) = 0;
      kernelName = boot->bpb.volumeLabel;
      bootDrive = driveID;
      DEB1("Boot sector @ 0x");
      DEB1(bwtoh(segVal));
      DEB1(" Boot drive = ");
      if(debug >= 1)
         biosChar(bootDrive + 'A');
      DEB1("\r\n");

   /* Load kernel at standard location */
      kernelSeg = KERNEL_SEG;
   }

/* set up some basic information for later */
   FATTableStart = boot->bpb.hiddenSectors + 1L;
   DEB2("FATTableStart = 0x");
   DEB2(bwtoh((WORD)(FATTableStart >> 16)));
   DEB2(bwtoh((WORD)FATTableStart));
   DEB2("\r\n");
   rootDirStart  = FATTableStart + (boot->bpb.sectorsPerFAT * boot->bpb.nFATs);
   DEB2("rootDirStart = 0x");
   DEB2(bwtoh((WORD)(rootDirStart >> 16)));
   DEB2(bwtoh((WORD)rootDirStart));
   DEB2("\r\n");
   fileAreaStart = rootDirStart + ((boot->bpb.rootDirSize * 32L) / 512L);
   DEB2("fileAreaStart = 0x");
   DEB2(bwtoh((WORD)(fileAreaStart >> 16)));
   DEB2(bwtoh((WORD)fileAreaStart));
   DEB2("\r\n");
   if(boot->bpb.extendedBPB == EXTENDED_BPB)
   {
      DEB2("DOS 4.x/5.x BPB\r\n");
      if(!boot->bpb.totalSize2)
      {
         DEB2("Volume >32MBytes\r\n");
         totalSectors = boot->bpb.totalSize4;
      }
      else
         totalSectors = boot->bpb.totalSize2;
   }
   else
   {
      DEB2("DOS 2.x/3.x BPB\r\n");
      totalSectors = boot->bpb.totalSize2;
   }
   DEB2("totalSectors = 0x");
   DEB2(bwtoh((WORD)(totalSectors >> 16)));
   DEB2(bwtoh((WORD)totalSectors));
   DEB2("\r\n");
   if(totalSectors / boot->bpb.sectorsPerCluster >= 4087L)
   {
      FATType = FAT16;
      DEB2("16-bit FAT entries\r\n");
   }

/* Say what's happening before using the disk drive :) */
   bputs("LiteLoad - Searching for kernel: ");
   for(i=0; i<8; i++)
      biosChar(kernelName[i]);
   biosChar('.');
   for(i=8; i<11; i++)
      biosChar(kernelName[i]);
   bputs("\r\n");

/* now attempt to find the kernel file in the root directory */
   kernelCluster = findKernel(kernelName);
   if(kernelCluster == 0 || kernelCluster >= RESV_CLUSTER(FATType))
   {
      bputs("Error, cannot find kernel file\r\n");
      return 1;
   }

/* identify the kernel type */
   switch(kernelType=identKernel(kernelCluster))
   {
   case KERNEL_16:
      bputs("Kernel is an MS-DOS .EXE format program\r\n");
      if(loadExe(kernelCluster, kernelSeg))
         return 1;
      break;
   case KERNEL_32:
      bputs("Kernel is a 32-bit COFF format program\r\n");
      if(loadCoff(kernelCluster, kernelSeg))
         return 1;
      break;
   default:
      bputs("Error, cannot identify the kernel type\r\n");
      return 1;
   }

/* last check for shift key pressed (user may have waited for 'Loading:') */
   if(biosKey(2) & 0x3)
      debug++;

/* Start kernel accordingly */
   if(KERNEL_16 == kernelType)
      startExe(kernelSeg);
   else
      startCoff(kernelSeg);
   return 0;
}

WORD findKernel(char far *kernelName)
{
DWORD currentSector = rootDirStart;
int entry;

/* search root directory for kernel file entry */
   while(currentSector < fileAreaStart)
   {
   /* load a sector of the directory */
      if(biosRead(bootDrive, currentSector, 1, &(boot->bpb), diskBuf.raw))
      {
         bputs("Error reading root directory\r\n");
         return 0;
      }

   /* Search it's entries */
      for(entry = 0; entry < DIR_SIZE; entry++)
      {
         if(bcmp(kernelName, diskBuf.dirent[entry].name,
               sizeof(diskBuf.dirent[entry].name)) == 0)
         {
            DEB2(" Size: 0x");
            DEB2(bwtoh( (WORD)(diskBuf.dirent[entry].size >> 16) ));
            DEB2(bwtoh( (WORD)(diskBuf.dirent[entry].size) ));
            DEB2(" Date: 0x");
            DEB2(bwtoh(diskBuf.dirent[entry].date));
            DEB2(" Time: 0x");
            DEB2(bwtoh(diskBuf.dirent[entry].time));
            DEB2(" @ Cluster: 0x");
            DEB2(bwtoh(diskBuf.dirent[entry].cluster));
            bputs("\r\n");
            return diskBuf.dirent[entry].cluster;
         }
      }

   /* step to next sector */
      currentSector++;
   }
   return 0;
}

int identKernel(WORD cluster)
{
DWORD logicalSector;

/* calculate the starting logical sector address of the cluster */
   logicalSector = (DWORD)boot->bpb.sectorsPerCluster * (DWORD)(cluster - 2)
                     + fileAreaStart;

/* load first sector of kernel file into disk buffer */
   if(biosRead(bootDrive, logicalSector, 1, &(boot->bpb), diskBuf.raw))
   {
      bputs("Error reading kernel file\r\n");
      return -1;
   }

/* See what it is! */
   if(*((WORD *)diskBuf.raw) == EXEMAGIC)
      return KERNEL_16;
   if(*((WORD *)diskBuf.raw) == I386MAGIC)
      return KERNEL_32;
   return -1;
}

int loadExe(WORD nextCluster, WORD kernelSeg)
{
char far *loadAddr;
WORD offset, bytes, skip, rel, far *relocation, far * far *relTable;

/* copy the .EXE header loaded by identKernel */
   bputs("Loading: ");
   exeHdr = *((exeHeader_t *)diskBuf.raw);

/* load the relocation table into memory *above* the program space */
   FP_SEG(relTable) = kernelSeg + (exeHdr.fileBlocks+1) * 512/16;
   FP_OFF(relTable) = 0;
   offset = exeHdr.offsetToRelocations;
   bytes = exeHdr.relocations * 4;
   DEB2("relTable = 0x");
   DEB2(bwtoh(FP_SEG(relTable)));
   DEB2(bwtoh(FP_OFF(relTable)));
   DEB2(" from offset: 0x");
   DEB2(bwtoh(offset));
   DEB2(" size: 0x");
   DEB2(bwtoh(bytes));
   biosChar('R');
   if(bytes)
   {
      if(read(&nextCluster, &offset, (char far *)relTable, bytes) != bytes)
      {
         bputs("Error loading .EXE relocation table.\r\n");
         return 1;
      }
   }

/* Skip rest of .EXE header */
   skip = bytes + exeHdr.offsetToRelocations;
   DEB2("skip = 0x");
   DEB2(bwtoh(skip));
   DEB2(" exeHdr size = 0x");
   DEB2(bwtoh(exeHdr.headerParagraphs * 16));
   biosChar('S');
   while(skip < exeHdr.headerParagraphs * 16)
   {
      if((exeHdr.headerParagraphs * 16 - skip) < boot->bpb.bytesPerSector)
         bytes = exeHdr.headerParagraphs * 16 - skip;
      else
         bytes = boot->bpb.bytesPerSector;
      if(read(&nextCluster, &offset, diskBuf.raw, bytes) != bytes)
      {
         bputs("Error skipping rest of EXE header\r\n");
         return 1;
      }
      skip += bytes;
      biosChar('.');
   }

/* Load the program into memory */
   FP_SEG(loadAddr) = kernelSeg;
   FP_OFF(loadAddr) = 0;
   DEB2("loadAddr = 0x");
   DEB2(bwtoh(FP_SEG(loadAddr)));
   DEB2(bwtoh(FP_OFF(loadAddr)));
   biosChar('L');
   while(read(&nextCluster, &offset, loadAddr, boot->bpb.bytesPerSector) > 0)
   {
      FP_SEG(loadAddr) += (boot->bpb.bytesPerSector >> 4);
      biosChar('.');
   }
   bputs("\r\n");

/* Pause if serious debugging */
   if(debug >= 2)
   {
      bputs("Paused, press a key..\r\n");
      biosKey(0);
   }

/* Re-locate the image file */
   bputs("Relocating: ");
   FP_SEG(loadAddr) = kernelSeg;
   FP_OFF(loadAddr) = 0;
   for(rel = 0; rel < exeHdr.relocations; rel++)
   {
      relocation = relTable[rel];
      FP_SEG(relocation) += FP_SEG(loadAddr);
      *relocation += FP_SEG(loadAddr);
      biosChar('.');
   }
   bputs("\r\n");
   return 0;
}

int loadCoff(WORD nextCluster, WORD loadSeg)
{
char far *pMem;
WORD offset, n, i;
char *wait = "-\\|/";
DWORD byteCnt;

/* Copy COFF header loaded by identKernel */
   bputs("Loading: ");
   coffHdr = *((coffHeader_t *)diskBuf.raw);
   offset = sizeof(coffHdr);

/* Check it out */
   if(!(coffHdr.coffHdr.f_flags & F_EXEC))
   {
      bputs("Error, COFF file is not executable\r\n");
      return 1;
   }
   if(coffHdr.coffHdr.f_opthdr != AOUTSZ)
   {
      bputs("Error, COFF file does not have a.out header\r\n");
      return 1;
   }
   if(coffHdr.coffHdr.f_nscns != 3)
   {
      bputs("Error, COFF file does not have 3 sections\r\n");
      return 1;
   }

/* load the .text section */
   i = 0;
   bputs(".text  ");
   for(byteCnt=0L; byteCnt<coffHdr.textScn.s_size; byteCnt+=(DWORD)n)
   {
      biosChar('\b');
      biosChar(wait[i]);
      i = (i+1)%4;
      if(coffHdr.textScn.s_size - byteCnt < (DWORD)boot->bpb.bytesPerSector)
         n = (WORD)(coffHdr.textScn.s_size - byteCnt);
      else
         n = boot->bpb.bytesPerSector;
      FP_SEG(pMem) = (WORD)((coffHdr.textScn.s_paddr + byteCnt) >> 4);
      FP_OFF(pMem) = (WORD)(coffHdr.textScn.s_paddr + byteCnt) % 16;
      if(read(&nextCluster, &offset, pMem, n) != n)
      {
         bputs("Error loading .text section\r\n");
         return 1;
      }
   }
   biosChar('\b');
   bputs(bwtoh((WORD)(byteCnt >> 16)));
   bputs(bwtoh((WORD)byteCnt));

/* load the .data section */
   bputs(" .data  ");
   for(byteCnt=0L; byteCnt<coffHdr.dataScn.s_size; byteCnt+=(DWORD)n)
   {
      biosChar('\b');
      biosChar(wait[i]);
      i = (i+1)%4;
      if(coffHdr.dataScn.s_size - byteCnt < (DWORD)boot->bpb.bytesPerSector)
         n = (WORD)(coffHdr.dataScn.s_size - byteCnt);
      else
         n = boot->bpb.bytesPerSector;
      FP_SEG(pMem) = (WORD)((coffHdr.dataScn.s_paddr + byteCnt) >> 4);
      FP_OFF(pMem) = (WORD)(coffHdr.dataScn.s_paddr + byteCnt) % 16;
      if(read(&nextCluster, &offset, pMem, n) != n)
      {
         bputs("Error loading .text section\r\n");
         return 1;
      }
   }
   biosChar('\b');
   bputs(bwtoh((WORD)(byteCnt >> 16)));
   bputs(bwtoh((WORD)byteCnt));

/* initialise the .bss section */
   bputs(" .bss  ");
   for(byteCnt=0L; byteCnt<coffHdr.bssScn.s_size; byteCnt++)
   {
      biosChar('\b');
      biosChar(wait[i]);
      i = (i+1)%4;
      FP_SEG(pMem) = (WORD)((coffHdr.bssScn.s_paddr + byteCnt) >> 4);
      FP_OFF(pMem) = (WORD)(coffHdr.bssScn.s_paddr + byteCnt) % 16;
      *pMem = 0;
   }
   biosChar('\b');
   bputs(bwtoh((WORD)(byteCnt >> 16)));
   bputs(bwtoh((WORD)byteCnt));
   bputs("\r\n");
   return 0;
}

int read(WORD *pc, WORD *po, char far *addr, WORD len)
{
int tcnt;
WORD so, cnt;
DWORD logSector;

/* Debugging */
   DEB2("read(");
   DEB2(bwtoh(*pc));
   DEB2(",");
   DEB2(bwtoh(*po));
   DEB2(",");
   DEB2(bwtoh(FP_SEG(addr)));
   DEB2(bwtoh(FP_OFF(addr)));
   DEB2(",");
   DEB2(bwtoh(len));
   DEB2(")=");

/* paranoia */
   if(!pc || !po || !addr || !len)
   {
      bputs("read(): Invalid parameters\r\n");
      return -1;
   }

/* read data from specified cluster & offset, update values */
   tcnt = 0;
   while(len)
   {
   /* Check cluster value */
      if(*pc == 0)
      {
         bputs("Invalid cluster value (0)\r\n");
         return -1;
      }
      if(*pc >= RESV_CLUSTER(FATType))
      {
         if(*pc == BAD_CLUSTER(FATType))
         {
            bputs("Bad cluster (");
            bputs(bwtoh(*pc));
            bputs(") in file\r\n");
            return -1;
         }
         else if(*pc >= LAST_CLUSTER(FATType))
         {
            break;
         }
         else
         {
            bputs("Reserved cluster (");
            bputs(bwtoh(*pc));
            bputs(") in file\r\n");
            return -1;
         }
      }

   /* read a sector into disk buffer */
      logSector = (DWORD)boot->bpb.sectorsPerCluster * (DWORD)(*pc - 2)
                  + fileAreaStart + (DWORD)(*po / boot->bpb.bytesPerSector);
      if(biosRead(bootDrive, logSector, 1, &(boot->bpb), diskBuf.raw))
      {
         bputs("Error reading sector 0x");
         bputs(bwtoh((WORD)(logSector >> 16)));
         bputs(bwtoh((WORD)logSector));
         bputs("\r\n");
         return -1;
      }

   /* Transfer data to specified location */
      so = *po % boot->bpb.bytesPerSector;
      for(cnt=0; cnt<len && (cnt+so)<boot->bpb.bytesPerSector; cnt++)
         addr[cnt] = diskBuf.raw[cnt+so];

   /* Update offsets & counters, possibly get next cluster number */
      len -= cnt;
      addr += cnt;
      tcnt += cnt;
      *po += cnt;
      if( *po >=
         (boot->bpb.bytesPerSector * (WORD)boot->bpb.sectorsPerCluster) )
      {
         *po = 0;
         *pc = nextFATCluster(*pc);
      }
   }
   DEB2(bwtoh(tcnt));
   DEB2("\r\n");
   return tcnt;
}

WORD nextFATCluster(WORD cluster)
{
DWORD nextFATSector, offset;
WORD index, nextCluster;

/* Calculate required offset into FAT */
   DEB2("<nFc(");
   DEB2(bwtoh(cluster));
   DEB2(")=");
   if(FATType == FAT12)
      offset = ((DWORD)cluster * 3L) / 2L;
   else
      offset = (DWORD)cluster * 2L;

/* See which FAT sectors are required (and the index into them) */
   nextFATSector = offset / 512L;
   index = (WORD)(offset % 512L);

/* see if we need to load it */
   if(FATSector != nextFATSector)
   {
      DEB2("<F>");
      FATSector = nextFATSector;
      if(biosRead(bootDrive, FATTableStart + nextFATSector,
         1, &(boot->bpb), FATBuf))
      {
         bputs("Error reading FAT sector 0x");
         bputs(bwtoh((WORD)(nextFATSector >> 16)));
         bputs(bwtoh((WORD)nextFATSector));
         bputs("\r\n");
         return BAD_CLUSTER(FATType);
      }
      nextFATSector++;
      if(biosRead(bootDrive, FATTableStart + nextFATSector,
         1, &(boot->bpb), &FATBuf[512]))
      {
         bputs("Error reading FAT sector 0x");
         bputs(bwtoh((WORD)(nextFATSector >> 16)));
         bputs(bwtoh((WORD)nextFATSector));
         bputs("\r\n");
         return BAD_CLUSTER(FATType);
      }
   }

/* Read FAT entry for required cluster */
   if(FATType == FAT12)
   {
      nextCluster =  (WORD)(FATBuf[index]);
      nextCluster |= (WORD)(FATBuf[index + 1]) << 8;
      if(cluster & 0x0001)
         nextCluster >>= 4;
      else
         nextCluster &= 0xFFF;
   }
   else
   {
      nextCluster =  (WORD)(FATBuf[index]);
      nextCluster |= (WORD)(FATBuf[index + 1]) << 8;
   }
   DEB2(bwtoh(nextCluster));
   DEB2(">");
   return nextCluster;
}

int startExe(WORD loadSeg)
{
char c, far *cmdTail;
int i;

/* set up the registers and jump to .EXE startup */
   DEB1("Starting kernel @ 0x");
   DEB1(bwtoh(exeHdr.offsetToCS + loadSeg));
   DEB1(bwtoh(exeHdr.initialIP));
   DEB1("\r\n");
   if(debug >= 1)
   {
      bputs("Enter startup arguments: ");
      FP_SEG(cmdTail) = loadSeg - 0x10;
      FP_OFF(cmdTail) = 0x80;
      i = 1;
      c = 0;
      while(i<127 && c != '\r' && c != '\n')
      {
         switch(c=(char)biosKey(0))
         {
         case '\t':
            cmdTail[i++] = c = ' ';
            break;
         case '\b':
            if(i>1)
               i--;
            else
               continue;
            break;
         default:
            cmdTail[i++] = c;
         }
         biosChar(c);
      }
      cmdTail[0] = (char)(i-1);
   }
   if(exeStart(&exeHdr, loadSeg))
   {
      bputs("Error, unable to start kernel.\r\n");
      return 1;
   }

/* Should never get to here.. */
   return 1;
}

int startCoff(WORD loadSeg)
{
descriptor_t gdt[3];
int i;

/* Create GDT entries and go! */
   DEB1("Starting kernel @ 0x");
   DEB1(bwtoh((WORD)(coffHdr.aoutHdr.entry >> 16)));
   DEB1(bwtoh((WORD)coffHdr.aoutHdr.entry));
   DEB1("L\r\n");
   if(debug >= 1)
   {
      bputs("Press a key..");
      biosKey(0);                   /* Wait for a key press if debugging ! */
   }
   for(i=0; i<sizeof(gdt[0]); i++)
      ((char *)&gdt[0])[i] = 0;
   codeSegment(&gdt[1], 0L, 0xFFFFFL);
   dataSegment(&gdt[2], 0L, 0xFFFFFL);
   loadGdt(sizeof(gdt)-1, gdt);
   goProt(8, coffHdr.aoutHdr.entry, 16, 16, STACK_32);

/* Should never get to here.. */
   return 1;
}

int bputs(char *str)
{
int cnt = 0;
char ch;

/* Write string to screen using BIOS calls */
   while(ch = *str++)
   {
      biosChar(ch);
      cnt++;
   }
   return cnt;
}

char *bwtoh(WORD w)
{
static char output[5];
WORD nibble;

/* convert WORD to Hexadecimal format string */
   nibble = (w & 0xF000) >> 12;
   if(nibble > 9)
      output[0] = (char)(nibble - 10 + 'A');
   else
      output[0] = (char)(nibble + '0');

   nibble = (w & 0x0F00) >> 8;
   if(nibble > 9)
      output[1] = (char)(nibble - 10 + 'A');
   else
      output[1] = (char)(nibble + '0');

   nibble = (w & 0x00F0) >> 4;
   if(nibble > 9)
      output[2] = (char)(nibble - 10 + 'A');
   else
      output[2] = (char)(nibble + '0');

   nibble = (w & 0x000F);
   if(nibble > 9)
      output[3] = (char)(nibble - 10 + 'A');
   else
      output[3] = (char)(nibble + '0');

   output[4] = 0;

   return output;
}

int bcmp(char far *s1, char far *s2, int bytes)
{
int cnt;

/* compare strings */
   for(cnt = 0; cnt < bytes; cnt++)
   {
      if(s1[cnt] > s2[cnt])
         return 1;
      if(s1[cnt] < s2[cnt])
         return -1;
   }
   return 0;
}

/* End */
