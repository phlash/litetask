# LITETASK Kernel BIOS driver make file

# $Author:   Phlash  $
# $Date:   27 Jan 1994 20:32:56  $
# $Revision:   1.1  $

include $(LITETASK)\kernel\makeopts

#--------------------------
# Top level dependancies...
#--------------------------
all : bios.lib

clean:
	del *.obj
	del *.lst
	del bios.lib
	del bios.bak
	del bios.pub

bios.lib : int15.obj bioscon.obj biosdisk.obj
	llib bios.lib bios.rsp

#--------------------------
# Int 15h multi-tasking handler
#--------------------------
int15.obj : int15.asm
	lasm $(AOPTS) $<

#--------------------------
# BIOS Keyboard/screen driver
#--------------------------
bioscon.obj : bioscon.c
	cl $(COPTS) $(DEBUG) bioscon.c

#--------------------------
# BIOS Disk driver
#--------------------------
biosdisk.obj : biosdisk.c
	cl $(COPTS) $(DEBUG) biosdisk.c

# End.
