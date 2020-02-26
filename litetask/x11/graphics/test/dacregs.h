/* DACREGS.H */

/* Prototypes for DAC register manipulation functions */

extern void far setDACregister(int pixel, unsigned long rgb);
extern void far setDACregisters(unsigned long far *ppalette);

/* End */
