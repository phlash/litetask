/*-----------------------------------------------------------------------
   CLIB.H - 'ANSI' C support library for Litetask Kernel

   $Author:$
   $Date:$
   $Revision:$

-----------------------------------------------------------------------*/

#ifndef _CLIB_H
#define _CLIB_H

/*-----------------------------------------------------------------------
   Compiler specific definitions
-----------------------------------------------------------------------*/

#ifdef M_I86      /* Microsoft C - 16-bit system */

#  ifndef M_I86LM
#  error Must compile with large model!
#  endif

/* memory object size type */
typedef unsigned long   size_t;

/* Maximum allowable alocation size */
#define MAX_ALLOC       0xFFF0

/* CPU register types */
typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;

/* CPU register set structures (for int86() function) */
#pragma pack(1)
typedef struct {
            WORD ax, bx, cx, dx;
            WORD ds, es, si, di;
            WORD flags;
            } WORDREGS;

typedef struct {
            BYTE al, ah, bl, bh, cl, ch, dl, dh;
            WORD ds, es, si, di;
            WORD flags;
            } BYTEREGS;

union REGS {
            WORDREGS x;
            BYTEREGS h;
           };
#pragma pack()

/* far pointer manipulation */
#define FP_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned *)&(fp)))

#endif   /* Microsoft C */

/*-----------------------------------------------------------------------
   Memory Management API
-----------------------------------------------------------------------*/

/* NULL pointer definition */
#define NULL ((void *)0)

/* initialise the heap manager */
extern void initHeap(char *heapStart, size_t heapSize);

/* allocate and free heap memory */
extern void *malloc(size_t size);
extern void *calloc(size_t number, size_t size);
extern int free(void *block);

/* copy, compare & set memory blocks */
extern void memcpy(void *dst, void *src, size_t size);
extern int memcmp(void *s1, void *s2, size_t size);
extern void memset(void *dst, char c, size_t size);

/* walk the heap/free lists */
typedef struct {
               size_t size;
               int used;
               int last;
               void *mcb;
               } heapInfo_t;
extern int walkHeap(heapInfo_t *phi);
extern int walkFree(heapInfo_t *phi);

/* tidy up the heap */
extern int tidyHeap(void);

/* memory buffer pool management */
typedef struct buf_tag {
            struct buf_tag *next;
            char data[1];
} buf_t;

typedef struct bufPool_tag {
            unsigned short free;
            buf_t *head, *tail;
} bufPool_t;

extern bufPool_t *createBufPool(unsigned short blocks, size_t size);
extern int resizeBufPool(bufPool_t *bufPool, unsigned short blocks);
extern int deleteBufPool(bufPool_t *bufPool);

/* allocate & free memory buffers (can be used in interrupt handlers) */
extern buf_t *getBuffer(bufPool_t *bufPool);
extern int putBuffer(bufPool_t *bufPool, buf_t *buffer);

/*-----------------------------------------------------------------------
   String Handling & Text Formatting API
-----------------------------------------------------------------------*/

extern char *strcpy(char *s1, char *s2);
extern char *strcat(char *s1, char *s2);
extern int strlen(char *s);
extern int strcmp(char *s1, char *s2);
extern char *strchr(char *s, char c);
extern char *strrchr(char *s, char c);
extern char * strstr(char *s1, char *s2);
extern int sprintf(char *buf, char *fmt, ...);
extern char getopts(char *cmd, char *opts, char **pOpArg);
extern void resetopts(void);

/* Function pointer type for formatted text output via doPrint() */
typedef void (*chOutFunc_t)(void *arg, int ch);

/* The formatting function itself */
extern int doPrint(chOutFunc_t pOut, void *arg, char *fmt, char *argp);

/*-----------------------------------------------------------------------
   BIOS Interface
-----------------------------------------------------------------------*/

/* text output (int 10h/ah=0Eh) support */
extern void biosCh(int c);
extern void biosStr(char *s);

/* keyboard (int 16h) support */
extern int biosKey(int op, ...);

#define BK_READ   0x0000      /* scan code in return high byte, ASCII in low */
#define BK_PEEK   0x0100      /* ditto, or zero for nothing waiting */
#define BK_FLAGS  0x0200      /* bitfield in return low byte (see below) */
#define BK_RATE   0x0305      /* arg 2 has delay in high byte, rate in low */
#define BK_NCLICK 0x0400      /* turn click off */
#define BK_CLICK  0x0401      /* turn click on */
#define BK_EREAD  0x1000      /* As above, but on extended keyboard */
#define BK_EPEEK  0x1100
#define BK_EFLAGS 0x1200      /* bitfield in return word (see below) */

#define BKF_RSHIFT   0x0001
#define BKF_LSHIFT   0x0002
#define BKF_CTRL     0x0004
#define BKF_ALT      0x0008
#define BKF_SCROLL   0x0010
#define BKF_NUM      0x0020
#define BKF_CAPS     0x0040
#define BKF_INSERT   0x0080
#define BKF_LCTRL    0x0100   /* only on extended keyboard from here on */
#define BKF_LALT     0x0200
#define BKF_RCTRL    0x0400
#define BKF_RALT     0x0800
#define BKF_SCROLLK  0x1000
#define BKF_NUMK     0x2000
#define BKF_CAPSK    0x4000
#define BKF_SYSREQ   0x8000

/* break key (int 1Bh) support */
extern void (*setBreakKeyTrap(void (*newBreak)(void)))(void);

/*-----------------------------------------------------------------------
   CPU Level operations
-----------------------------------------------------------------------*/

/* port IO */
extern void outp(int port, unsigned char byte);
extern void outpw(int port, unsigned short word);
extern unsigned char inp(int port);
extern unsigned short inpw(int port);

/* setjmp & longjmp */
#define _JBLEN 9
typedef int jmp_buf[_JBLEN];
extern int setjmp(int *);
extern void longjmp(int *, int);

/* interrupt vector stuff */
extern void *setVector(int vector, void *address);
extern int setIRQTrap(int irq, void *function, void *stacktop);
extern int clearIRQTrap(int irq);

/* interrupt invoke stuff */
extern int int86(short intr, union REGS *regs);
extern int chainIRQ(int irq);

/* interrupt control */
extern void enableInts(void);
extern void disableInts(void);
extern short lockInts(void);
extern void unlockInts(short flag);

/* debugger support */
typedef struct {
         WORD ES,DS,BP,DI,SI,DX,CX,BX,AX,IP,CS,FLAGS;
         } debugRegs_t;

#define FLAGBITS  0x0FD5
#define FL_OF     0x0800
#define FL_DF     0x0400
#define FL_IF     0x0200
#define FL_TF     0x0100
#define FL_SF     0x0080
#define FL_ZF     0x0040
#define FL_AF     0x0010
#define FL_PF     0x0004
#define FL_CF     0x0001

extern void setDebuggerTrap(void (*newTrap)(int, debugRegs_t *));

/*-----------------------------------------------------------------------
   Useful macros
-----------------------------------------------------------------------*/

#define min(a, b) ( ((a)>(b)) ? (b) : (a) )
#define max(a, b) ( ((a)<(b)) ? (b) : (a) )

#endif
/* End */
