@echo off
rem * getkbuf.bat $Revision:$
rem * Gets the kernel message buffer into a file using debug :-)

set KFILE=kerntest.msg
if not %1x == x set KFILE=%1

set KADDR=9000:0
if not %2x == x set KADDR=%2

rem Create response file
echo n%KFILE% > dbg.rsp
echo rcx >>dbg.rsp
echo 7FFF >>dbg.rsp
echo w %KADDR% >>dbg.rsp
echo q >>dbg.rsp

rem * Run it through debug
debug <dbg.rsp
del dbg.rsp

