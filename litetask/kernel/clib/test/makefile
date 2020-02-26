# Make file for LiteTask C Library test program

include $(LITETASK)\kernel\makeopts

.c.obj:
   cl $(COPTS) $<

All: clibtest.exe

clean:
   del *.obj
   del clibtest.exe

clibtest.exe: clibtest.obj
   link ..\..\cstart $* ..\..\miscglue,$*,$*.map /m,..\clib.lib;
