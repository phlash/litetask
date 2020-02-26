/* ========================= External Routines ========================= */
extern int near biosChar(char ch);
extern int near biosKey(BYTE function);
extern int near biosRead(BYTE drive, DWORD logSector,
                         WORD nSectors, BPB far *bpb,
                         char far *buffer);

/* NB: By supplying a NULL value for the BPB pointer, biosRead() will use
   logSector as the head/cyl/sector value directly, instead of calculating
   it. Store the head in byte 3, cyl/sector in BIOS format in lower word. */
