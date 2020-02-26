@echo off
echo Making LiteTask (%1)..

echo o Bootup..
cd bootup
pmake %1
cd ..

echo o Kernel..
cd kernel
pmake %1
cd ..

echo o Drivers..
cd drivers
pmake %1
cd ..

echo o File Systems..
cd filesys
pmake %1
cd ..

echo o X11 Library..
cd x11
pmake %1
cd ..

echo o Utility Programs..
cd utils
pmake %1
cd ..

echo Done!
