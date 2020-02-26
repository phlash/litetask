/* Function pointer type for formatted text output via doPrint() */
typedef void (far *chOutFunc_t)(void far *arg, int ch);

/* The formatting function itself */
int far doPrint(chOutFunc_t pOut, void far *arg, char far *fmt, char far *argp);

/* End */
