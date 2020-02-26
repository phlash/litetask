# LITETASK Kernel drivers make file

# $Author:   Phlash  $
# $Date:   08 May 1994 15:43:26  $
# $Revision:   1.10  $

include $(LITETASK)\kernel\makeopts

DIR_LIST=bios serial mouse

#--------------------------
# Top level dependancies...
#--------------------------
all : iosys.obj select.obj $(DIR_LIST)
	llib drivers.lib drivers.rsp

clean:
	del *.obj
	del drivers.lib
	del drivers.bak
	del drivers.pub
	pmake MAKE_TYPE=clean $(DIR_LIST)

#--------------------------
# IO System
#--------------------------
iosys.obj : iosys.c
	cl $(COPTS) $(DEBUG) iosys.c

select.obj : select.c
	cl $(COPTS) $(DEBUG) select.c

#--------------------------
# Driver bits...
#--------------------------
$(DIR_LIST) : FRC
	cd $@
	pmake $(MAKE_TYPE)
	cd ..

FRC :

# End.
