/* DTABLE.H - 386(tm) DX Descriptor table strucutres  */

#ifndef _DTABLE_H
#define _DTABLE_H

/* Basic CPU types */
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned long   ulong;

/*
 * Descriptor table (GDT/LDT/IDT) entry structures
 */

/* That all important access rights byte */
#define ACCESS_BYTE  uchar  d_stype: 4; \
                     uchar  d_dtype: 1; \
                     uchar  d_dpl:   2; \
                     uchar  d_pres:  1

/* Memory segment descriptor */
typedef struct {
            ushort d_lim1;
            ushort d_base1;

            uchar  d_base2;

            ACCESS_BYTE;

            uchar  d_lim2:  4;
            uchar  d_avail: 1;
            uchar  d_zero:  1;
            uchar  d_big:   1;
            uchar  d_gran:  1;

            uchar  d_base3;
} dt_seg;

/* Control gate descriptor */
typedef struct {
            ushort d_off1;
            ushort d_seg;
            uchar  d_resv;
            ACCESS_BYTE;
            ushort d_off2;
} dt_gate;

typedef union {
            uchar    d_raw[8];
            dt_seg   d_seg;
            dt_gate  d_gate;
} descriptor_t;


/*
 * Bitfield values for access rights byte
 */

/* Descriptor Types in d_dtype */
#define D_DT_SYSTEM  0
#define D_DT_APPLIC  1

/* system Segment/gate Types in d_stype (when d_dtype == D_DT_SYSTEM) */
/* NB: 0,8,10,13 are reserved by Intel */
#define D_ST_2TSS    1     /* 286 Task State Segment */
#define D_ST_LDT     2     /* Local Descriptor Table segment */
#define D_ST_2TSSB   3     /* 286 TSS, Busy */
#define D_ST_2CALL   4     /* 286 Call gate */
#define D_ST_TASK    5     /* Task gate */
#define D_ST_2INT    6     /* 286 Interrupt gate */
#define D_ST_2TRAP   7     /* 286 Trap gate */
#define D_ST_3TSS    9     /* 386 Task State Segment */
#define D_ST_3TSSB   11    /* 386 TSS, Busy */
#define D_ST_3CALL   12    /* 386 Call gate */
#define D_ST_3INT    14    /* 386 Interrupt gate */
#define D_ST_3TRAP   15    /* 386 Trap gate */

/* application Segment Type bits in d_stype (when d_dtype == D_DT_APPLIC) */
/* OR together either ST_CODE/ST_DATA and bits from either STC/STD sections */
#define D_ST_DATA    0     /* A data segment */
#define D_ST_CODE    8     /* A code segment */

#define D_STD_EXPDN  4     /* Expand down segment (for stacks) */
#define D_STD_WRITE  2     /* Writable */
#define D_STD_ACCESS 1     /* Accessed bit */

#define D_STC_CONF   4     /* Conforming code segment */
#define D_STC_READ   2     /* Readable */
#define D_STC_ACCESS 1     /* Accessed bit */

/* Descriptor privilege levels in d_dpl (ZERO = maximum privilege) */
#define D_DPL_ZERO   0
#define D_DPL_ONE    1
#define D_DPL_TWO    2
#define D_DPL_THREE  3

/* Segment present bit (for memory segments) */
#define D_PRESENT    1


/*
 * Descriptor Table register structure (for LGDT/LLDT/LIDT instructions)
 */

typedef struct {
            ushort dt_limit;
            ulong  dt_addr;
            } dtable_t;


/*
 * Convert a table index and protection level to a selector value 
 */
#define dtIndexToSelector(i,p) ((i)<<3 | ((p)&3)<<1)


/*
 * Support functions in DTABLE.C 
 */
void limitAndBase(descriptor_t *pd, ulong base, ulong limit);
void codeSegment(descriptor_t *pd, ulong base, ulong limit);
void dataSegment(descriptor_t *pd, ulong base, ulong limit);

#endif

/* End */
