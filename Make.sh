#!/bin/bash
##
## Make.sh: build script for debuggerBIOS-ng-ng (C++ edition)
##
## Needs the Vircon32 DevTools (compile, assemble, packrom, png2vircon,
## wav2vircon) and v32c++ (the C++ to Vircon32 C transpiler) in the PATH.
##
## v32c++ finds the DevTools headers (audio.h, video.h, ...) in the include/
## folder next to `compile`, or in /usr/local/Vircon32/DevTools/include. If
## yours are somewhere else, set V32CXX_SDK_INCLUDE to that folder. To use
## headers from other places (the v32c++ ones, say) set V32CXX_INCLUDE.
##
########################################################################################

########################################################################################
##
## define an abort function to call on error
##
abort_build()
{
    echo
    echo BUILD FAILED
    exit 1
}

########################################################################################
##
## create obj and bin folders if non exiting, since
## the development tools will not create them themselves
##
mkdir -p obj
mkdir -p bin

########################################################################################
##
## C++ to Vircon32 C. A BIOS is built with -b, which tells v32c++ to leave
## error_handler() with its plain name (the compiler looks for that exact name).
##
## --no-signed-shift: the source only shifts values it has masked, and the plain
## shift is what the CPU does natively (a logical one), at a lower cost.
##
echo
echo Transpile the C++ code
echo --------------------------
v32c++ -b -g --no-signed-shift --no-build-scripts debuggerBIOS-ng.cpp -o obj/debuggerBIOS-ng.c \
    || abort_build

########################################################################################
##
## compilation of a BIOS requires argument -b
##
echo
echo Compile the C code
echo --------------------------
compile  -g         obj/debuggerBIOS-ng.c   -o obj/debuggerBIOS-ng.asm  -b || abort_build

########################################################################################
##
## assembly of a BIOS requires argument -b
##
echo
echo Assemble the ASM code
echo --------------------------
assemble -g program obj/debuggerBIOS-ng.asm -o obj/debuggerBIOS-ng.vbin -b || abort_build

########################################################################################
##
## Convert the PNG to VTEX
##
echo
echo Convert the PNG texture
echo --------------------------
png2vircon          BiosTexture.png -o obj/BiosTexture.vtex   || abort_build

########################################################################################
##
## Convert the WAV to VSND
##
echo
echo Convert the WAV sound
echo --------------------------
wav2vircon          BiosSound.wav   -o obj/BiosSound.vsnd     || abort_build

########################################################################################
##
## pack up all the processed assets into the final BIOS ROM
##
echo
echo Pack the ROM
echo --------------------------
packrom             obj/debuggerBIOS-ng.xml    -o bin/debuggerBIOS-ng.v32 || abort_build

########################################################################################
##
## If we get this far, the build is presumed successful
##
echo
echo BUILD SUCCESSFUL
