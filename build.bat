@echo off

set CompilerFlags=-MTd -Gm- -GR- -EHa- -nologo -Oi -FC -Z7 -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -wd4101 -wd4366 -wd4701
set LinkerFlags= -INCREMENTAL:NO -opt:ref User32.lib Opengl32.lib Gdi32.lib Winmm.lib

IF NOT EXIST ".\build" mkdir ".\build"
pushd ".\build"

if "%1" == "debug" goto debug

:release
cl /Fe:"program.exe" %CompilerFlags% -Oi -O2 -DNO_ASSERTS ..\code\main.cpp /link %LinkerFlags% 
goto end

:debug
cl /Fe:"program.exe" %CompilerFlags% -Od ..\code\main.cpp /link %LinkerFlags%

:end
popd

REM Do the manifest thing to disable dpi scaling.
mt.exe -manifest ".\code\program.exe.manifest" -outputresource:".\build\program.exe;1" -nologo