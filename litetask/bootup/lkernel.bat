@echo off
rem *** LKERNEL.BAT - Load a LiteTask .EXE or COFF kernel as if the machine
rem *** was booted from specified drive. Assumes all drives on BIOS disk 80

if "%1" == "" goto usage
if "%2" == "" goto usage
if %1 == c set DRN=2
if %1 == C set DRN=2
if %1 == d set DRN=3
if %1 == D set DRN=3
if "%DRN%" == "" goto usage

rem *** copy the kernel file to the root directory of specified drive
copy %2 %1:\LITETASK.EXE

rem *** Create the debug response file (eek!)
if exist LK_DBG.RSP del LK_DBG.RSP

echo l9000:0 %DRN% 0 1>>LK_DBG.RSP
echo a9000:2b>>LK_DBG.RSP
echo db 'LITETASKEXE'>>LK_DBG.RSP
echo.>>LK_DBG.RSP
echo mcs:100 lf000 9010:100>>LK_DBG.RSP
echo rss>>LK_DBG.RSP
echo 8fc0>>LK_DBG.RSP
echo rsp>>LK_DBG.RSP
echo 400>>LK_DBG.RSP
echo rds>>LK_DBG.RSP
echo 9000>>LK_DBG.RSP
echo rdx>>LK_DBG.RSP
echo 80>>LK_DBG.RSP
echo rcs>>LK_DBG.RSP
echo 9010>>LK_DBG.RSP
echo r>>LK_DBG.RSP
echo g>>LK_DBG.RSP

rem *** Now run it!
debug liteload.com <LK_DBG.RSP
goto end

:usage
echo usage: lkernel {C/D} {kernel file}

:end
