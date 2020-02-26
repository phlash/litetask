#include "dos_disk.h"
#include "loadutil.h"

#define NULL   0L

int indent;

void biosStr(char *str)
{
   while(*str)
      biosChar(*str++);
}

void biosByte(BYTE b)
{
BYTE nibble;

   nibble = (b >> 4) & 0xF;
   biosChar( (nibble > 9) ? nibble-10+'A' : nibble+'0' );
   nibble = b & 0xF;
   biosChar( (nibble > 9) ? nibble-10+'A' : nibble+'0' );
}

void biosWord(WORD w)
{
   biosByte( (BYTE)(w >> 8) );
   biosByte( (BYTE)w );
}

void biosDword(DWORD d)
{
   biosWord( (WORD)(d >> 16) );
   biosWord( (WORD)d );
}

void biosDecByte(BYTE b)
{
   if(b>99) biosChar( (b/100) + '0' );
   if(b>9) biosChar( ((b%100)/10) + '0' );
   biosChar( (b%10) + '0' );
}

void biosDecWord(WORD w)
{
   if(w>9999) biosChar( (BYTE)(w/10000) + '0' );
   if(w>999) biosChar( (BYTE)((w%10000)/1000) + '0' );
   if(w>99) biosChar( (BYTE)((w%1000)/100) + '0' );
   if(w>9) biosChar( (BYTE)((w%100)/10) + '0' );
   biosChar( (BYTE)(w%10) + '0' );
}

void displayBpb(union diskSector far *pds)
{
int i;

/* Validity check */
   if(pds->bpb.bytesPerSector != 512)
   {
      biosStr("Non-DOS boot sector\r\n");
      return;
   }
   biosStr("BPB: OEM=\"");
   for(i=0;i<8;i++)
      biosChar(pds->bpb.oemId[i]);
   biosStr("\" Label=\"");
   for(i=0;i<11;i++)
      biosChar(pds->bpb.volumeLabel[i]);
   biosStr("\" cluster=");
   biosDecByte(pds->bpb.sectorsPerCluster);
   biosStr(" FATs=");
   biosDecByte(pds->bpb.nFATs);
   biosStr("@");
   biosDecWord(pds->bpb.sectorsPerFAT);
   biosStr(" root=");
   biosDecWord(pds->bpb.rootDirSize);
   biosStr("\r\n");
}

void displayPart(union diskSector far *pds, int i)
{
int j;

   biosStr("P[");
   biosByte((BYTE)i);
   biosStr("]: ");
   if(!pds->ptable.partition[i].type)
   {
      biosStr("Unused\r\n");
      return;
   }
   biosStr("A=");
   biosByte(pds->ptable.partition[i].active);
   biosStr(" T=");
   biosByte(pds->ptable.partition[i].type);
   biosStr(" S(C/H/S)=");
   biosDecWord( CS_CYL(pds->ptable.partition[i].startCylSector) );
   biosChar('/');
   biosDecByte(pds->ptable.partition[i].startHead);
   biosChar('/');
   biosDecWord( CS_SEC(pds->ptable.partition[i].startCylSector) );
   biosStr(" E=");
   biosDecWord( CS_CYL(pds->ptable.partition[i].endCylSector) );
   biosChar('/');
   biosDecByte(pds->ptable.partition[i].endHead);
   biosChar('/');
   biosDecWord( CS_SEC(pds->ptable.partition[i].endCylSector) );
   biosStr(" O=");
   biosDword(pds->ptable.partition[i].offset);
   biosStr(" L=");
   biosDword(pds->ptable.partition[i].length);
   biosStr("\r\n");
   for(j=0; j<indent; j++) biosChar(' ');
   biosStr(" Type: ");
   switch(pds->ptable.partition[i].type)
   {
   case PT_DOS12:
      biosStr("DOS/12bit FAT\r\n");
      break;
   case PT_XENIX:
      biosStr("Xenix\r\n");
      break;
   case PT_DOS16:
      biosStr("DOS/16bit FAT\r\n");
      break;
   case PT_EXTEND:
      biosStr("DOS/Extended\r\n");
      break;
   case PT_BIGDOS:
      biosStr("DOS/16bit FAT > 32MBytes\r\n");
      break;
   case PT_NOVELL:
      biosStr("Novell\r\n");
      break;
   case PT_PCIX:
      biosStr("PC/IX\r\n");
      break;
   case PT_EXT2:
      biosStr("Linux/EXT II\r\n");
      break;
   case PT_386BSD:
      biosStr("386/BSD\r\n");
      break;
   case PT_CPM:
      biosStr("CP/M\r\n");
      break;
   case PT_BBT:
      biosStr("BBT\r\n");
      break;
   default:
      biosStr("Unknown\r\n");
      break;
   }
}

int analyseSector(BYTE drive, union diskSector far *pds, int part)
{
union diskSector ds;
DWORD physSector;
WORD rv;
int p, i;

/* Analyse */
   if(!part)
   {
   /* It's a BPB */
      for(i=0; i<indent; i++) biosChar(' ');
      displayBpb(pds);
      return 0;
   }

   for(p=0; p<4; p++)
   {
      for(i=0; i<indent; i++) biosChar(' ');
      displayPart(pds, p);
      switch(pds->ptable.partition[p].type)
      {
      case PT_DOS12:
      case PT_DOS16:
      case PT_BIGDOS:
      case PT_EXTEND:
         physSector = pds->ptable.partition[p].startCylSector |
                ((DWORD)pds->ptable.partition[p].startHead << 16);
         if(rv=biosRead(drive, physSector, 1, NULL, ds.raw))
         {
            biosStr("Error reading disk: ");
            biosWord(rv);
            return -1;
         }
         indent+=2;
         if(pds->ptable.partition[p].type == PT_EXTEND)
            analyseSector(drive, &ds, 1);
         else
            analyseSector(drive, &ds, 0);
         indent-=2;
      }
   }
   return 0;
}

int main()
{
union diskSector ds;
WORD rv;
BYTE drive;

/* Prompt for disk drive */
   biosStr("Enter drive letter: ");
   drive = (BYTE)biosKey(0);
   if(drive <'a') drive = drive + ('a' - 'A');
   biosChar(drive);
   biosStr("\r\n");
   drive = (drive > 'b') ? drive-'c'+0x80 : drive-'a';

/* Read first disk sector */
   if(rv=biosRead(drive, 1L, 1, NULL, ds.raw))
   {
      biosStr("Error reading disk: ");
      biosWord(rv);
      return 1;
   }

/* Analyse it! */
   indent=0;
   return analyseSector(drive, &ds, (drive & 0x80) ? 1 : 0);
}

