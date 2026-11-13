@echo off
pushd %~dp0\..\
REM Forwards extra Premake flags, e.g. --with-vulkan
call vendor\bin\premake5.exe vs2019 %*
set ERR=%ERRORLEVEL%
popd
if %ERR% neq 0 exit /b %ERR%
PAUSE
