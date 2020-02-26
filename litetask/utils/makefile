# Makefile for LiteTask utilities

all: lprof.exe

clean:
	del lprof.obj
	del lprof.exe

lprof.exe : lprof.obj
	link /noi lprof;

lprof.obj : lprof.c
	cl /AL /W3 /c lprof.c

# End.
