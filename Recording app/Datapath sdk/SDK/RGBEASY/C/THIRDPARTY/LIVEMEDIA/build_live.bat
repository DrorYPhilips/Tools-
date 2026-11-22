@echo off

echo Please ensure that you have read README.txt before running this script
pause
title build_live

:: Change this to the correct location.
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86

:: Change this to the correct location.
set liveRoot="D:\SDK\RGBEASY\C\THIRDPARTY\LIVEMEDIA\live"

:::::::::::::::::::::::::::::::::::::

cd %liveRoot%
call genWindowsMakefiles

cd %liveRoot%\BasicUsageEnvironment
del *.obj *.lib
nmake /B -f BasicUsageEnvironment.mak
if errorlevel 1 goto broken

cd %liveRoot%\groupsock
del *.obj *.lib
nmake /B -f groupsock.mak
if errorlevel 1 goto broken

cd %liveRoot%\liveMedia
del *.obj *.lib
nmake /B -f liveMedia.mak
if errorlevel 1 goto broken

cd %liveRoot%\UsageEnvironment
del *.obj *.lib
nmake /B -f UsageEnvironment.mak
if errorlevel 1 goto broken

goto end
:::::::::::::::::::::::::::::::::::::
:broken
echo Help
pause

:end