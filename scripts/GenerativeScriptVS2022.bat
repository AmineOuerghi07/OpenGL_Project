@echo off
pushd %~dp0\..\
REM Forwards extra Premake flags, e.g.:
REM   GenerativeScriptVS2022.bat --with-vulkan
REM   GenerativeScriptVS2022.bat --arch=x64
call vendor\bin\premake5.exe vs2022 %*
set ERR=%ERRORLEVEL%
popd
if %ERR% neq 0 exit /b %ERR%
PAUSE
