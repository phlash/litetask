# Make file for LiteTask kernel test programs

include $(LITETASK)\kernel\makeopts

.c.obj:
   cl $(COPTS) $<

All: kerntest.exe

clean:
   del *.exe
   del *.obj
   del *.map
   del *.msg

kerntest.exe: kerntest.obj
   link $(LOPTS) $*,$*,$*.map /m,..\litetask.lib

# End
