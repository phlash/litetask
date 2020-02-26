# LITETASK Kernel Serial port driver make file

# $Author:   Phlash  $
# $Date:   26 Apr 1994 21:10:28  $
# $Revision:   1.0  $

include $(LITETASK)\kernel\makeopts

#--------------------------
# Top level dependancies...
#--------------------------
all : serial.lib

clean:
	del *.obj
	del serial.lib
	del serial.bak
	del serial.pub

serial.lib : serial.obj
	llib serial.lib serial.rsp

#--------------------------
# Serial port driver
#--------------------------
serial.obj : serial.c
	cl $(COPTS) $(DEBUG) serial.c

# End.
