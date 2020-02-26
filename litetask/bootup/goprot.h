/* GOPROT.H - Prototypes for protected mode support functions (GOPROT.ASM) */

extern unsigned long loadGdt(unsigned short limit, void far *gdtAddr);
extern unsigned long loadIdt(unsigned short limit, void far *idtAddr);
extern void goProt(unsigned short cs, unsigned long eip,
                   unsigned short ds,
                   unsigned short ss, unsigned long esp);

/* End */
