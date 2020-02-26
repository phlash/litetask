#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <bios.h>

/* position of secondary loader name in boot sector */
#define LOADER_NAME  0x1F0

/* position of kernel file name in boot sector */
#define KERNEL_NAME  0x2B

/* buffer for boot sector data */
static char boot_sector[512];

void usage(void)
{
      puts("usage: boot_sec <r|w|m|s|k> <drive> [<file name>]");
      puts("where r = read boot sector into file");
      puts("      w = write boot sector from file");
      puts("      m = merge boot sector BPB & file and write to boot sector");
      puts("      s = show/set secondary loader file name in boot sector");
      puts("      k = show/set kernel file name in boot sector");
}

int doBoot(int drive, int rw)
{
union REGS regs;
struct SREGS sregs;
struct diskfree_t df;
struct {
   unsigned long sector;
   unsigned short count;
   unsigned short off;
   unsigned short seg;
   } dp;
void far *p;

/* read drive info */
   if(_dos_getdiskfree(drive+1, &df))
   {
      perror("reading drive information ");
      return -1;
   }
   if( ((long)df.total_clusters * (long)df.sectors_per_cluster) >
      65536L )
   {
      regs.h.al = drive;
      regs.x.cx = -1;
      p = (void far *)&dp;
      regs.x.bx = FP_OFF(p);
      sregs.ds = FP_SEG(p);
      dp.sector = 0L;
      dp.count = 1;
      p = (void far *)boot_sector;
      dp.off = FP_OFF(p);
      dp.seg = FP_SEG(p);
   }
   else
   {
      regs.h.al = drive;
      regs.x.cx = 1;
      regs.x.dx = 0;
      p = (void far *)boot_sector;
      regs.x.bx = FP_OFF(p);
      sregs.ds = FP_SEG(p);
   }
   if(rw)
      int86x(0x26, &regs, &regs, &sregs);
   else
      int86x(0x25, &regs, &regs, &sregs);
   if(regs.x.cflag)
   {
      perror("accessing boot sector");
      return -1;
   }
   return 0;
}

void doRead(int drive, int fd)
{
   printf("Reading boot sector from drive %c:\n", drive+'A');

/* read boot sector */
   if(doBoot(drive, 0))
      return;

/* now write out to file */
   write(fd, boot_sector, sizeof(boot_sector));
}

void doWrite(int drive, int fd)
{
   printf("Writing boot sector to drive: %c\n", drive+'A');

/* read in from file into memory buffer */
   read(fd, boot_sector, sizeof(boot_sector));

/* write boot sector */
   doBoot(drive, 1);
}

void doMerge(int drive, int fd)
{
int i;
char temp_sector[512];

   printf("Merging boot sector onto drive: %c\n", drive+'A');

/* read file into temporary buffer */
   read(fd, temp_sector, sizeof(temp_sector));

/* read boot sector into buffer */
   if(doBoot(drive, 0))
      return;

/* merge BPB into file data */
   for(i=11; i<43; i++)
      temp_sector[i] = boot_sector[i];

/* transfer file data to buffer */
   memcpy(boot_sector, temp_sector, sizeof(boot_sector));

/* now write out to boot sector of disk */
   doBoot(drive, 1);
}

void doName(int drive, int offset, char *name)
{
int i;

   printf("Updating name on drive %c: to %s\n", drive+'A', name);

/* read boot sector into memory buffer */
   if(doBoot(drive, 0))
      return;

/* merge file name at given offset (add white space where necessary) */
   i = 0;
   while(i < 8 && *name && *name != '.')
   {
      boot_sector[offset+i] = *name++;
      i++;
   }
   while(i < 8)
   {
      boot_sector[offset+i] = ' ';
      i++;
   }
   if(*name == '.')
   {
      name++;
      while(i < 11 && *name)
      {
         boot_sector[offset+i] = *name++;
         i++;
      }
   }
   while(i < 11)
   {
      boot_sector[offset+i] = ' ';
      i++;
   }

/* now write out to boot sector of disk */
   doBoot(drive, 1);
}

void showName(int drive, int offset)
{
int i;

/* read boot sector into memory buffer */
   if(doBoot(drive, 0))
      return;

/* display name at specified offset */
   printf("Name is: ");
   for(i=0; i<8; i++)
      putchar(boot_sector[offset+i]);
   putchar('.');
   for(; i<11; i++)
      putchar(boot_sector[offset+i]);
   putchar('\n');
}

void main(int argc, char **argv)
{
int drive, fd;
char *name;

/* check args */
   if(argc < 3)
   {
      usage();
      return;
   }


/* select drive */
   drive = toupper(argv[2][0]) - 'A';

/* select required operation */
   switch(argv[1][0])
   {
   case 'w':
   case 'W':
      if(argc < 4)
      {
         usage();
         return;
      }
      if((fd = open(argv[3], O_BINARY | O_RDONLY)) == -1)
      {
         perror("opening data file ");
         return;
      }
      doWrite(drive, fd);
      close(fd);
      break;
   case 'r':
   case 'R':
      if(argc < 4)
      {
         usage();
         return;
      }
      if((fd = open(argv[3], O_BINARY | O_WRONLY | O_CREAT, 0666)) == -1)
      {
         perror("opening data file ");
         return;
      }
      doRead(drive, fd);
      close(fd);
      break;
   case 'm':
   case 'M':
      if(argc < 4)
      {
         usage();
         return;
      }
      if((fd = open(argv[3], O_BINARY | O_RDONLY)) == -1)
      {
         perror("opening data file ");
         return;
      }
      doMerge(drive, fd);
      close(fd);
      break;
   case 's':
   case 'S':
      if(argc < 4)
         showName(drive, LOADER_NAME);
      else
         doName(drive, LOADER_NAME, strupr(argv[3]));
      break;
   case 'k':
   case 'K':
      if(argc < 4)
         showName(drive, KERNEL_NAME);
      else
         doName(drive, KERNEL_NAME, strupr(argv[3]));
      break;
   default:
      usage();
      break;
   }
}

