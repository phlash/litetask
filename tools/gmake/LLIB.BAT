@echo off
rem Wrapper for LIB because GNUmake doesn't get on with COMMAND syntax

del %1
lib %1;
lib @%2
