/* DACREGS.H */

/* Prototypes for DAC register manipulation functions */

extern void far setDACregister(short pixel, unsigned long rgb);
extern void far setDACregisters(unsigned long far *rgbs, short start, short nregs);

/* End */
