# LITETASK Kernel Mouse driver make file

# $Author:   Phlash  $
# $Date:   03 Jun 1994 20:09:00  $
# $Revision:   1.0  $

include $(LITETASK)\kernel\makeopts

#--------------------------
# Top level dependancies...
#--------------------------
all : mouse.lib

clean:
	del *.obj
	del mouse.lib
	del mouse.bak
	del mouse.pub

mouse.lib : mouse.obj
	llib mouse.lib mouse.rsp

#--------------------------
# Mouse driver
#--------------------------
mouse.obj : mouse.c
	cl $(COPTS) $(DEBUG) mouse.c

# End.
