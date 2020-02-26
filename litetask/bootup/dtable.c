/* DTABLE.C - 386(tm) DX Descriptor table support functions */

#include "dtable.h"

void limitAndBase(descriptor_t *pd, ulong base, ulong limit)
{
   /* Segment limit & base */
   pd->d_seg.d_lim1  = (ushort)limit;
   pd->d_seg.d_lim2  = (uchar)(limit >> 16);
   pd->d_seg.d_base1 = (ushort)base;
   pd->d_seg.d_base2 = (uchar)(base >> 16);
   pd->d_seg.d_base3 = (uchar)(base >> 24);
   /* 32-bit options */
   pd->d_seg.d_avail = 0;
   pd->d_seg.d_zero  = 0;
   pd->d_seg.d_big   = 1;
   pd->d_seg.d_gran  = 1;
}

void codeSegment(descriptor_t *pd, ulong base, ulong limit)
{
/* fill out a GDT code segment entry */
   /* Access rights byte */
   pd->d_seg.d_dtype = D_DT_APPLIC;
   pd->d_seg.d_stype = D_ST_CODE | D_STC_CONF | D_STC_READ;
   pd->d_seg.d_dpl   = D_DPL_ZERO;
   pd->d_seg.d_pres  = D_PRESENT;
   limitAndBase(pd, base, limit);
}

void dataSegment(descriptor_t *pd, ulong base, ulong limit)
{
/* fill out a GDT data segment entry */
   /* Access rights byte */
   pd->d_seg.d_dtype = D_DT_APPLIC;
   pd->d_seg.d_stype = D_ST_DATA | D_STD_WRITE;
   pd->d_seg.d_dpl   = D_DPL_ZERO;
   pd->d_seg.d_pres  = D_PRESENT;
   limitAndBase(pd, base, limit);
}

/* End */
