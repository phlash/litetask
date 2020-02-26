# Makefile for Litetask Kernel C Library

# $Author:   Phlash  $
# $Date:   16 Jul 1995  9:14:12  $
# $Revision:   1.13  $

include $(LITETASK)\kernel\makeopts

OBJS=memory.obj \
     bufpool.obj \
     setjmp.obj \
     bioslib.obj \
     textdrv.obj \
     string.obj \
     sprintf.obj \
     printk.obj \
     doprint.obj \
     semaphor.obj \
     getopts.obj

all : clib.lib

clean:
	del clib.lib
	del clib.bak
	del clib.pub
	del *.obj

clib.lib : $(OBJS) clib.rsp
	llib clib.lib clib.rsp

.c.obj:
	cl $(COPTS) $(DEBUG) $<

.asm.obj:
	lasm $(AOPTS) $<

# End.
