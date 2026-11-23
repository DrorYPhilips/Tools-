@echo off
setlocal
set "PATH=%LOCALAPPDATA%\Programs\msys64\ucrt64\bin;%PATH%"
set "APP=%~dp0build\FrameCheck.exe"
if not exist "%APP%" (
    echo The updated application was not found: "%APP%"
    echo Build it using the instructions in README.md.
    pause
    exit /b 1
)
start "" "%APP%"
