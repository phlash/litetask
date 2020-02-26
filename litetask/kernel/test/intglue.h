extern int far setIRQTrap(int irq, void far *func, void far *intStack);
extern int far clearIRQTrap(int irq);
extern int far chainIRQ(int irq);
extern int far overrunIRQ(int irq);
