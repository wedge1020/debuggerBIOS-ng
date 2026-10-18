@echo off
rem Make.bat: build script for debuggerBIOS (C++ edition)
rem Needs the Vircon32 DevTools and v32c++ in the PATH.
rem If the DevTools headers are not next to `compile`, set V32CXX_SDK_INCLUDE.

if not exist obj mkdir obj
if not exist bin mkdir bin

echo.
echo Transpile the C++ code
echo --------------------------
v32c++ -b -g --no-signed-shift --no-build-scripts debuggerBIOS.cpp -o obj\debuggerBIOS.c
if errorlevel 1 goto failed

echo.
echo Compile the C code
echo --------------------------
compile -g obj\debuggerBIOS.c -o obj\debuggerBIOS.asm -b
if errorlevel 1 goto failed

echo.
echo Assemble the ASM code
echo --------------------------
assemble -g program obj\debuggerBIOS.asm -o obj\debuggerBIOS.vbin -b
if errorlevel 1 goto failed

echo.
echo Convert the PNG texture
echo --------------------------
png2vircon assets\BiosTexture.png -o obj\BiosTexture.vtex
if errorlevel 1 goto failed

echo.
echo Convert the WAV sound
echo --------------------------
wav2vircon assets\BiosSound.wav -o obj\BiosSound.vsnd
if errorlevel 1 goto failed

echo.
echo Pack the ROM
echo --------------------------
packrom debuggerBIOS.xml -o bin\debuggerBIOS.v32
if errorlevel 1 goto failed

echo.
echo BUILD SUCCESSFUL
goto end

:failed
echo.
echo BUILD FAILED
exit /b 1

:end
