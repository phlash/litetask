/* DOS_DISK.H - MS-DOS Disk data structures */

/* Some basic types */
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;

/* All these structures are packed together */
#pragma pack(1)

/* HD Partition table structure */
typedef struct {
         char resv[446];
         struct {
            BYTE active;
            BYTE startHead;
            WORD startCylSector;
            BYTE type;
            BYTE endHead;
            WORD endCylSector;
            DWORD offset;
            DWORD length;
         } partition[4];
         BYTE signature[2];
} PTABLE;

/* Partition types (so far..) */
#define PT_DOS12  0x1
#define PT_XENIX  0x2
#define PT_DOS16  0x4
#define PT_EXTEND 0x5
#define PT_BIGDOS 0x6
#define PT_NOVELL 0x64
#define PT_PCIX   0x75
#define PT_EXT2   0x83
#define PT_386BSD 0xA5
#define PT_CPM    0xDB
#define PT_BBT    0xFF

/* Macros to manipulate xxxCylSector values */
#define MK_CS(c, s)  ( (((c) >> 2)&0xC0) | ((s)&0x3F) | ((c) << 8) )
#define CS_CYL(cs)   ( (((cs) << 2)&0x300) | (((cs) >> 8)&0xFF) )
#define CS_SEC(cs)   ( (cs)&0x3F )

/* BPB structure */
typedef struct {
         BYTE jump[3];            /* MS-DOS 2.0 and above */
         char oemId[8];
         WORD bytesPerSector;
         BYTE sectorsPerCluster;
         WORD reservedSectors;
         BYTE nFATs;
         WORD rootDirSize;
         WORD totalSize2;
         BYTE mediaByte;
         WORD sectorsPerFAT;

         WORD sectorsPerTrack;    /* MS-DOS 3.0 and above */
         WORD nHeads;
         DWORD hiddenSectors;

         DWORD totalSize4;        /* MS-DOS 4.0 and above */
         BYTE driveByte;          /* not to be relied upon with floppies! */
         BYTE resv1;
         BYTE extendedBPB;
         DWORD volumeId;
         char volumeLabel[11];
         char resv2[8];
} BPB;

#define EXTENDED_BPB   0x29

/* directory entries & directory sector(s) */
typedef struct {
         char name[11];
         BYTE attribute;
         char resv1[10];
         WORD time;
         WORD date;
         WORD cluster;
         DWORD size;
         } DIRENT;

#define DIR_SIZE (512 / sizeof(DIRENT))

union diskSector {
         PTABLE ptable;
         BPB bpb;
         DIRENT dirent[DIR_SIZE];
         char raw[512];
         };

#pragma pack()

/* FAT table types */
#define FAT12        0
#define FAT16        1

/* FAT table entries (all take a FAT type argument) */
#define FAT_SIZE(ft)       (((ft) == FAT12) ? 12 : 16)
#define FREE_CLUSTER(ft)   (0x0000)
#define RESV_CLUSTER(ft)   (((ft) == FAT12) ? 0xFF0 : 0xFFF0)
#define BAD_CLUSTER(ft)    (((ft) == FAT12) ? 0xFF7 : 0xFFF7)
#define LAST_CLUSTER(ft)   (((ft) == FAT12) ? 0xFF8 : 0xFFF8)

/* End */
