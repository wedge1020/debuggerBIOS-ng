#!/bin/bash

# define an abort function to call on error
abort_build()
{
    echo
    echo BUILD FAILED
    exit 1
}

# create obj and bin folders if non exiting, since
# the development tools will not create them themselves
mkdir -p obj
mkdir -p bin

echo
echo Compile the C code
echo --------------------------
# compilation of a BIOS requires argument -b
compile -g debuggerBIOS-ng.c -o obj/debuggerBIOS-ng.asm -b || abort_build

echo
echo Assemble the ASM code
echo --------------------------
# assembly of a BIOS requires argument -b
assemble -g program obj/debuggerBIOS-ng.asm -o obj/debuggerBIOS-ng.vbin -b || abort_build

echo
echo Convert the PNG texture
echo --------------------------
png2vircon BiosTexture.png -o obj/BiosTexture.vtex || abort_build

echo
echo Convert the WAV sound
echo --------------------------
wav2vircon BiosSound.wav -o obj/BiosSound.vsnd || abort_build

echo
echo Pack the ROM
echo --------------------------
packrom debuggerBIOS-ng.xml -o bin/debuggerBIOS-ng.v32 || abort_build

echo
echo BUILD SUCCESSFUL
