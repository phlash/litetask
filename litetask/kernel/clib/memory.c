/*------------------------------------------------------------------------
   MEMORY.C - Memory management routines for LiteTask O/S

   $Author:   Phlash  $
   $Date:   19 Feb 1995 18:13:00  $
   $Revision:   1.5  $

NOTE: Still uses lockTask() call to protect the heap consistancy from
      pre-emptive scheduling....MUST BE FIXED.
------------------------------------------------------------------------*/

#include <litetask.h>

/*
 * "tuneable" parameter for controlling block fragmentation. Default is not
 * to fragment a block if < 64 paragraphs (1k) would remain
 */
#define MIN_BLOCK_PARAGRAPHS  64

typedef struct _MCB_tag {
               char magic[4];
               unsigned short size;       /* In sizeof(_MCB) units */
               unsigned short flag;
               struct _MCB_tag *next;     /* Free list pointers */
               struct _MCB_tag *prev;
               } _MCB;
/*
 * values for memory control block flags
 */
#define MFREE_BLOCK   0
#define MLAST_BLOCK   0x8000
#define MUSED_BLOCK   0x0001

static _MCB *firstMCB = NULL;
static _MCB *freeList = NULL;
static char MCB_MAGIC[] = "LT-M";
                       
/*
 * routine called by main() to set heap parameters
 */
void initHeap(char *heapStart, size_t heapSize)
{
/* store address of first memory block, and configure it */
   firstMCB = heapStart;
   freeList = firstMCB;
   memcpy(firstMCB->magic, MCB_MAGIC, 4);
   firstMCB->size = (unsigned short)(heapSize/sizeof(_MCB))-1;
   firstMCB->flag = MLAST_BLOCK;
   firstMCB->next = NULL;
   firstMCB->prev = NULL;
}

/* 
 * walkHeap() - walks the heap, one step from the supplied starting point.
 * if phi->mcb == NULL, start at the beginning.
 * NB: This routines assumes that the heap will NOT be changed by any
 * other task(s) during the scan (or it may well fail a check).
 */
int walkHeap(heapInfo_t *phi)
{
_MCB *mcb;

/* get MCB pointer */
   if(phi->mcb)
      mcb = (_MCB *)phi->mcb;
   else
      mcb = firstMCB;
   
/* check magic field */
   if(memcmp(mcb->magic, MCB_MAGIC, 4))
   /* Corrupted heap detected */
      return EBADMCB;

/* assign size & flags */
   phi->size = (size_t)mcb->size * (size_t)sizeof(_MCB);
   if(mcb->flag & MUSED_BLOCK)
      phi->used = 1;
   else
      phi->used = 0;
   if(mcb->flag & MLAST_BLOCK)
      phi->last = 1;
   else
      phi->last = 0;

/* step to next block */
   if(mcb->flag & MLAST_BLOCK)
      mcb = NULL;
   else
      mcb += mcb->size + 1;
   phi->mcb = mcb;

/* all done */
   return 0;
}

/* 
 * walkFree() - walk the free list, one step from supplied starting point.
 * if phi->mcb == NULL, start at the beginning.
 * NB: This routines assumes that the heap will NOT be changed by any
 * other task(s) during the scan (or it may well fail a check).
 */
int walkFree(heapInfo_t *phi)
{
_MCB *mcb;

   if(phi->mcb)
      mcb = (_MCB *)phi->mcb;
   else
      mcb = freeList;

/* check magic field */
   if(memcmp(mcb->magic, MCB_MAGIC, 4))
   /* Corrupted heap detected */
      return EBADMCB;

/* assign size & flags */
   phi->size = (size_t)mcb->size * (size_t)sizeof(_MCB);
   if(mcb->flag & MUSED_BLOCK)
      phi->used = 1;
   else
      phi->used = 0;
   if(mcb->flag & MLAST_BLOCK)
      phi->last = 1;
   else
      phi->last = 0;

/* step to next block (unless allocated) */
   if(mcb->flag & MUSED_BLOCK)
      phi->mcb = NULL;
   else
      phi->mcb = mcb->next;

/* all done */
   return 0;
}

/* 
 * tidyHeap() - collect free blocks together and check heap consistancy,
 * this routine should be called regularly to clean up the heap.
 */
int tidyHeap(void)
{
_MCB *mcb, *next;
short flag;

/* lock task before playing with the heap.. */
   flag = lockTask();
   mcb = freeList;
   while(mcb != NULL)
   {
   /* check magic field */
      if(memcmp(mcb->magic, MCB_MAGIC, 4))
      {
      /* Corrupted heap detected, return non-zero to caller */
         unlockTask(flag);
         return EBADMCB;
      }

   /* see if this block reachs to the next free block */
      next = mcb + mcb->size + 1;
      if(next == mcb->next)
      {
      /* yep, so delete next MCB */
         mcb->size += next->size + 1;
         if(next->flag & MLAST_BLOCK)
            mcb->flag |= MLAST_BLOCK;
         next->magic[0] = 0;

      /* remove next MCB from free list */
         mcb->next = next->next;
         if(next->next)
            next->next->prev = mcb;
      }

   /* step to next block */
      mcb = mcb->next;
   }

/* all done */
   unlockTask(flag);
   return 0;
}

void *malloc(size_t size)
{
_MCB *mcb, *newmcb;
unsigned short bl_size;
short flag;

/* calculate nearest block size from that given */
   bl_size = (size+sizeof(_MCB)-1)/sizeof(_MCB);

/* lock task while playing with the heap */
   flag = lockTask();

/* fit requested block into a free area */
   mcb = freeList;
   while(mcb != NULL)
   {
      if(mcb->size >= bl_size)
         break;
      mcb = mcb->next;
   }

/* did we run out of memory? */
   if(mcb == NULL)
   {
      unlockTask(flag);
      return NULL;
   }

/* mark block as used */
   mcb->flag |= MUSED_BLOCK;

/* fragment block if possible, and adjust free list */
   if(bl_size + MIN_BLOCK_PARAGRAPHS < mcb->size)
   {
      newmcb = mcb+bl_size+1;
      memcpy(newmcb->magic, MCB_MAGIC, 4);
      newmcb->size = mcb->size - bl_size - 1;
      newmcb->flag = MFREE_BLOCK;
      newmcb->next = NULL;
      newmcb->prev = NULL;
      mcb->size = bl_size;
      if(mcb->flag & MLAST_BLOCK)
      {
         mcb->flag &= ~MLAST_BLOCK;
         newmcb->flag |= MLAST_BLOCK;
      }
      newmcb->next = mcb->next;
      newmcb->prev = mcb->prev;
      if(mcb->next)
         mcb->next->prev = newmcb;
      if(mcb->prev)
         mcb->prev->next = newmcb;
      else
         freeList = newmcb;
   }
   else
   {
   /* just adjust free list */
      if(mcb->next)
         mcb->next->prev = mcb->prev;
      if(mcb->prev)
         mcb->prev->next = mcb->next;
      else
         freeList = mcb->next;
   }

/* unlock task */
   unlockTask(flag);

/* return pointer to data block */
   return mcb+1;
}


int free(void *block)
{
_MCB *mcb, *before, *after;
short flag;

/* calculate & check MCB address */
   mcb = ((_MCB *)block) - 1;

/* check magic value */
   if(memcmp(mcb, MCB_MAGIC, 4))
      return EBADMCB;

/* mark block as free */
   flag = lockTask();
   mcb->flag &= ~MUSED_BLOCK;

/* now place at correct point in free list */
   after = freeList;
   before = NULL;
   while(after && after < mcb)
   {
      before = after;
      after = after->next;
   }
   mcb->next = after;
   mcb->prev = before;
   if(before)
      before->next = mcb;
   else
      freeList = mcb;
   if(after)
      after->prev = mcb;
   unlockTask(flag);
   return 0;
}

void *calloc(size_t number, size_t size)
{
size_t total=number*size, cnt;
char *newptr;

/* check for overflow */
   if(total > MAX_ALLOC)
      return NULL;

/* allocate it */
   if((newptr = malloc(total)) == NULL)
      return NULL;

/* fill it with zeros */
   for(cnt = 0; cnt < total; cnt++)
      newptr[cnt] = 0;

/* return it */
   return newptr;
}


void memcpy(void *dst, void *src, size_t length)
{
char *Dst=dst, *Src=src;
size_t cnt;

   for(cnt = 0; cnt < length; cnt++)
      Dst[cnt] = Src[cnt];
}

int memcmp(void *s1, void *s2, size_t length)
{
char *S1=s1, *S2=s2;
size_t cnt;

/* compare each element, return zero if the same, non-zero otherwise */
   for(cnt = 0; cnt < length; cnt++)
      if(S1[cnt] != S2[cnt])
         return 1;
   return 0;
}

void memset(void *dst, char c, size_t length)
{
size_t cnt;

   for(cnt=0; cnt<length; cnt++)
      ((char *)dst)[cnt] = c;
}

/* End. */
