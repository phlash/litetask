# LITETASK X11 graphics drivers library make file

# $Author:   Phlash  $
# $Date:   30 Jun 1994 19:11:10  $
# $Revision:   1.1  $

include $(LITETASK)\kernel\makeopts

#--------------------------
# Top level dependancies...
#--------------------------
all : graphics.lib

clean:
	del *.obj
	del graphics.lib
	del graphics.bak
	del graphics.pub

graphics.lib : vm18drv.obj vm19drv.obj dacregs.obj
	llib graphics.lib graphics.rsp

#--------------------------
# vm18drv -
# 640x480x16 video mode primitives
#--------------------------
vm18drv.obj : vm18drv.c prims.h colors.h dacregs.h
	cl $(COPTS) $(DEBUG) vm18drv.c

#--------------------------
# vm19drv -
# 320x200x256 video mode primitives
#--------------------------
vm19drv.obj : vm19drv.c prims.h colors.h dacregs.h
	cl $(COPTS) $(DEBUG) vm19drv.c

#--------------------------
# DacRegs - Video DAC register manipulation
#--------------------------
dacregs.obj : dacregs.asm
	lasm $(AOPTS) dacregs.asm

# End.
