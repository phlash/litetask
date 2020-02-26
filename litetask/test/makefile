# LITETASK Test code make file

COPTS=/c /W3 /Od /Gs /Zl /I..\include
LOPTS=/nod /noi /m /co

.c.obj:
   cl $(COPTS) $(DEBUG) $<

.obj.exe:
   link $(LOPTS) $<,$*.exe,$*.map, ..\kernel\litetask.lib

#--------------------------
# Top level dependancies...
#--------------------------
all : maintask.exe semtest.exe contest.exe biostest.exe \
      heaptest.exe xlibt.exe primst.exe sertest.exe \
      msetest.exe xlogo.exe

#--------------------------
# Test bits...
#--------------------------
maintask.exe : maintask.obj ..\kernel\litetask.lib \
   ..\x11\graphics\graphics.lib
   link $(LOPTS) maintask,maintask.exe,maintask.map, \
   ..\kernel\litetask.lib ..\x11\graphics\graphics.lib;

maintask.obj : maintask.c
   cl $(COPTS) $(DEBUG) maintask.c

contest.exe : contest.obj ..\kernel\litetask.lib ..\drivers\drivers.lib
   link $(LOPTS) contest,contest.exe,contest.map,\
   ..\kernel\litetask.lib ..\drivers\drivers.lib;

contest.obj : contest.c
   cl $(COPTS) $(DEBUG) contest.c

semtest.exe : semtest.obj ..\kernel\litetask.lib ..\filesys\filesys.lib
   link $(LOPTS) semtest,semtest.exe,semtest.map,..\kernel\litetask.lib ..\filesys\filesys.lib;

semtest.obj : semtest.c
   cl $(COPTS) $(DEBUG) semtest.c

biostest.exe : biostest.obj ..\kernel\litetask.lib ..\drivers\drivers.lib
   link $(LOPTS) biostest,biostest.exe,biostest.map,\
   ..\kernel\litetask.lib ..\drivers\drivers.lib;

biostest.obj : biostest.c
   cl $(COPTS) $(DEBUG) biostest.c

heaptest.exe : heaptest.obj ..\kernel\litetask.lib
   link $(LOPTS) heaptest rand,heaptest.exe,heaptest.map,..\kernel\litetask.lib;

heaptest.obj : heaptest.c
   cl $(COPTS) $(DEBUG) heaptest.c

xlibt.exe : xlibt.obj ..\kernel\litetask.lib \
            ..\x11\xlib\xlib.lib \
            ..\x11\graphics\graphics.lib \
            ..\filesys\filesys.lib
   link $(LOPTS) xlibt,,, ..\kernel\litetask ..\x11\xlib\xlib \
   ..\x11\graphics\graphics ..\filesys\filesys;

xlibt.obj : xlibt.c
   cl $(COPTS) $(DEBUG) xlibt.c

xlogo.exe : xlogo.obj ..\kernel\litetask.lib \
            ..\x11\xlib\xlib.lib \
            ..\x11\graphics\graphics.lib \
            ..\filesys\filesys.lib
   link $(LOPTS) xlogo,,, ..\kernel\litetask ..\x11\xlib\xlib \
   ..\x11\graphics\graphics ..\filesys\filesys;

xlogo.obj : xlogo.c
   cl $(COPTS) $(DEBUG) xlogo.c

primst.exe : primst.obj ..\kernel\litetask.lib ..\x11\graphics\graphics.lib
   link $(LOPTS) primst,primst.exe,primst.map,\
   ..\kernel\litetask.lib ..\x11\graphics\graphics.lib;

primst.obj : primst.c
   cl $(COPTS) $(DEBUG) primst.c

drawit.exe : drawit.obj mouse.obj ..\kernel\litetask.lib \
             ..\x11\xlib\xlib.lib
   link $(LOPTS) drawit mouse,drawit.exe,drawit.map,\
   ..\kernel\litetask.lib ..\x11\xlib\xlib.lib;

drawit.obj : drawit.c
   cl $(COPTS) $(DEBUG) drawit.c

mouse.obj : mouse.asm
   masm /mx mouse;

sertest.exe : sertest.obj ..\kernel\litetask.lib ..\drivers\drivers.lib
   link $(LOPTS) sertest,sertest.exe,sertest.map,\
   ..\kernel\litetask.lib ..\drivers\drivers.lib;

sertest.obj : sertest.c
   cl $(COPTS) $(DEBUG) sertest.c

terminal.exe : terminal.obj ..\kernel\litetask.lib ..\drivers\drivers.lib
   link $(LOPTS) terminal,terminal.exe,terminal.map,\
   ..\kernel\litetask.lib ..\drivers\drivers.lib;

terminal.obj : terminal.c
   cl $(COPTS) $(DEBUG) terminal.c

msetest.exe : msetest.obj ..\kernel\litetask.lib ..\drivers\drivers.lib
   link $(LOPTS) msetest,msetest.exe,msetest.map,\
   ..\kernel\litetask.lib ..\drivers\drivers.lib;

msetest.obj : msetest.c
   cl $(COPTS) $(DEBUG) msetest.c

printkt.exe : printkt.obj ..\kernel\litetask.lib
   link $(LOPTS) printkt,printkt.exe,printkt.map,..\kernel\litetask.lib;

printkt.obj : printkt.c
   cl $(COPTS) $(DEBUG) printkt.c

fattest.exe : fattest.obj ..\kernel\litetask.lib ..\filesys\filesys.lib
   link $(LOPTS) fattest,fattest.exe,fattest.map,..\kernel\litetask.lib ..\filesys\filesys.lib;

fattest.obj : fattest.c
   cl $(COPTS) $(DEBUG) fattest.c

proftest.exe: proftest.obj ..\kernel\litetask.lib ..\filesys\filesys.lib
   link $(LOPTS) proftest,proftest.exe,proftest.map,..\kernel\litetask ..\filesys\filesys
# End.
