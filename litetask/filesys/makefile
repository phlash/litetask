# LITETASK Kernel file systems make file

# $Author:   Phlash  $
# $Date:   05 Feb 1995 22:30:58  $
# $Revision:   1.0  $

include $(LITETASK)\kernel\makeopts

#--------------------------
# Top level dependancies...
#--------------------------
all : filesys.obj fat.obj int24.obj
	llib filesys.lib filesys.rsp

#--------------------------
# File System
#--------------------------
filesys.obj : filesys.c
	cl $(COPTS) $(DEBUG) filesys.c

#--------------------------
# FAT (MS-DOS) driver
#--------------------------
fat.obj : fat.c
	cl $(COPTS) $(DEBUG) fat.c

#--------------------------
# DOS INT 24h handler
#--------------------------
int24.obj: int24.asm
	lasm $(AOPTS) $<

clean:
	del filesys.lib
	del filesys.bak
	del filesys.pub
	del *.obj

# End.
