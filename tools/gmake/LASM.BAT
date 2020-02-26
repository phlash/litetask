@echo off
rem Wrapper for MASM because shit doesn't work from GNUmake -> COMMAND

echo "LASM: %1 %2"
masm %1 %2;
