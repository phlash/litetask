# Makefile for LiteTask 16/32-bit secondary boot loader LITELOAD.COM

# Tools
CC = cl
AS = masm
LD = link

# LiteTask standard options for make
include $(LITETASK)\kernel\makeopts

# Implicit rules
.c.obj:
	$(CC) /c /W3 $(DEBUG) $<

.asm.obj:
	$(AS) /mx $(DEBUG) $*,,$*;

.obj.exe:
	$(CC) $(DEBUG) $<

default: liteload.com boot_sec.exe bootup.bin
others: diskinf.com loadcoff.exe fileinfo.exe

clean:
	del *.exe
	del *.com
	ren anlmul.obj anlmul.prt
	ren anuldiv.obj anuldiv.prt
	del *.obj
	ren anlmul.prt anlmul.obj
	ren anuldiv.prt anuldiv.obj
	del *.lst
	del *.map
	del bootup.bin

liteload.com : comstart.obj \
               liteload.obj \
               loadutil.obj \
               exestart.obj \
               dtable.obj \
               goprot.obj
	$(LD) /noi /nod comstart liteload loadutil exestart dtable goprot anuldiv anlmul,\
      liteload.exe,liteload.map /map;
	exe2bin liteload.exe liteload.com
	del liteload.exe

boot_sec.exe: boot_sec.c
	$(CC) boot_sec.c

bootup.bin: bootup.obj
	$(LD) /noi bootup.obj,bootup.exe,bootup.map;
	exe2bin bootup.exe bootup.bin
	del bootup.exe

diskinf.com: comstart.obj diskinf.obj loadutil.obj
	$(LD) /noi /nod comstart.obj diskinf.obj loadutil.obj,diskinf.exe,diskinf.map /map;
	exe2bin diskinf.exe diskinf.com
	del diskinf.exe

loadcoff.exe: loadcoff.obj dtable.obj goprot.obj
	$(LD) /noi loadcoff dtable goprot,loadcoff.exe,loadcoff.map /map;

fileinfo.exe: fileinfo.obj

liteload.obj: exe.h coff.h dtable.h goprot.h
loadcoff.obj: coff.h dtable.h goprot.h
fileinfo.obj: coff.h
 
dtable.obj: dtable.h

# End
