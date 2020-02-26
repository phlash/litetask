# LITETASK X11 system make file

# $Author:   Phlash  $
# $Date:   14 Jun 1994 18:16:32  $
# $Revision:   1.1  $

include $(LITETASK)\kernel\makeopts

DIR_LIST=graphics xlib

#--------------------------
# Top level dependancies...
#--------------------------
all : $(DIR_LIST)

clean:
	pmake MAKE_TYPE=clean $(DIR_LIST)

#--------------------------
# Sub-dir bits...
#--------------------------
$(DIR_LIST) : FRC
	cd $@
	pmake $(MAKE_TYPE)
	cd ..

FRC :

# End.
