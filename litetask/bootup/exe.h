/* EXE.H - MS-DOS .EXE file header */

/* .EXE file header */
#pragma pack(1)
typedef struct
{
   BYTE exeSig[2];
   WORD lastBlockSize;
   WORD fileBlocks;
   WORD relocations;
   WORD headerParagraphs;
   WORD minAllocation;
   WORD maxAllocation;
   WORD offsetToSS;
   WORD initialSP;
   WORD checksum;
   WORD initialIP;
   WORD offsetToCS;
   WORD offsetToRelocations;
   WORD overlayNumber;
} exeHeader_t;
#pragma pack()

#define EXEMAGIC  0x5A4D

extern int near exeStart(exeHeader_t far *exeHeader, unsigned short loadSeg);

/* End */
