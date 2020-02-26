# LITETASK Xlib library make file

# $Author:   Phlash  $
# $Date:   09 Mar 1995 20:58:28  $
# $Revision:   1.2  $

include $(LITETASK)\kernel\makeopts

INCLUDES= /I ../graphics

#--------------------------
# Compiler rules
#--------------------------
.c.obj:
	cl $(COPTS) $(INCLUDES) $(DEBUG) $<

#--------------------------
# Top level dependancies...
#--------------------------
all : xlib.lib

clean:
	del *.obj
	del xlib.lib
	del xlib.bak
	del xlib.pub

xlib.lib : xlib.obj xwindow.obj xevents.obj xcolors.obj xdrawing.obj xroot.obj xutil.obj
	llib xlib.lib xlib.rsp

#--------------------------
# Xlib - main library code
#--------------------------
#xlib.c: xint.h $(LITETASK)\include\xlib.h ..\graphics\colors.h

#--------------------------
# XWindow - window routines
#--------------------------
#xwindow.obj : xwindow.c xint.h $(LITETASK)\include\xlib.h ..\graphics\colors.h

#--------------------------
# XEvents - event routines
#--------------------------
#xevents.obj : xevents.c xint.h $(LITETASK)\include\xlib.h

#--------------------------
# XColors - colormap routines
#--------------------------
#xcolors.obj : xcolors.c xint.h $(LITETASK)\include\xlib.h

#--------------------------
# XDrawing - drawing routines
#--------------------------
#xdrawing.obj : xdrawing.c xint.h $(LITETASK)\include\xlib.h

#--------------------------
# XRoot - default root window handler
#--------------------------
#xroot.obj : xroot.c xint.h $(LITETASK)\include\xlib.h

#--------------------------
# XUtil - Utility routines
#--------------------------
#xutil.obj : xutil.c xint.h $(LITETASK)\include\xlib.h

# End.
