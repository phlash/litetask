@echo off
if %1x == x set TARGET=A:
if not %1x == x set TARGET=%1

echo Exporting LiteTask kernel to %TARGET%

echo Creating directories..
echo on
mkdir %TARGET%\include
mkdir %TARGET%\lib
mkdir %TARGET%\bootup
mkdir %TARGET%\misc
@echo off

echo Copying header files..
copy include\*.h %TARGET%\include /b/v

echo Copying kernel library..
copy kernel\litetask.lib %TARGET%\lib /b/v >NUL

echo Copying device driver library..
copy drivers\drivers.lib %TARGET%\lib /b/v >NUL

echo Copying X11 library..
copy x11\xlib\xlib.lib %TARGET%\lib /b/v >NUL

echo Copying bootup support files..
copy bootup\bootup.bin %TARGET%\bootup /b/v >NUL
copy bootup\boot_sec.exe %TARGET%\bootup /b/v >NUL
copy bootup\liteload.com %TARGET%\bootup /b/v >NUL

:nextarg
shift
if %1x == x goto done

echo Copying additional files: %1..
echo on
copy %1 %TARGET%\misc /b/v
@echo off
goto nextarg

:done
echo Done.
set TARGET=
