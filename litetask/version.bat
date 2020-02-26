@echo off
if %1x == x goto usage
set LABEL=%1
shift
:next
if %1x == x goto label
set LABEL=%LABEL% %1
shift
goto next
:label
echo Labelling LiteTask source code at version %LABEL%..
dotoall vcs -v"%LABEL%" *.??v
goto end
:usage
echo Usage: version {version string}
:end
