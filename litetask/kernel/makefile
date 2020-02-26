# LITETASK Kernel make file

# $Author:   Phlash  $
# $Date:   24 Jun 1995 20:15:32  $
# $Revision:   1.7  $

# NB: You must set the environment variable 'LITETASK' before running this
# makefile.

include $(LITETASK)\kernel\makeopts

#--------------------------
# Object list
#--------------------------
OBJS= cstart.obj \
      main.obj \
      task.obj \
      timer.obj \
      idle.obj \
      traps.obj \
      debug.obj \
      intglue.obj \
      trapglue.obj \
      taskglue.obj \
      miscglue.obj

#--------------------------
# Compile rules
#--------------------------
.c.obj:
	cl $(COPTS) $<

.asm.obj:
	lasm $(AOPTS) $<

#--------------------------
# Top level dependancies...
#--------------------------
all : $(OBJS) clib
	llib litetask.lib litetask.rsp

clean:
	del *.obj
	del *.lst
	pmake MAKE_TYPE=clean clib
	del litetask.lib
	del litetask.bak
	del litetask.pub

clib : FRC
	cd clib
	pmake $(MAKE_TYPE)
	cd ..

FRC :

# End.
