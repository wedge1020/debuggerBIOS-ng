#!/usr/bin/env bash
###
### Make.sh - build script for inline test investigation
###
########################################################################################

########################################################################################
##
## generate the VTEX
##
echo -n "[vtex] "
png2vircon          -o BiosTexture.vtex BiosTexture.png && echo "ok" || echo "fail"

########################################################################################
##
## generate the VSND
##
echo -n "[vsnd] "
wav2vircon          -o BiosSound.vsnd   BiosSound.wav   && echo "ok" || echo "fail"

########################################################################################
##
## compile the C to assembly
##
echo -n "[c]    "
compile  -g         -o inline.asm       inline.c        && echo "ok" || echo "fail"

########################################################################################
##
## assemble the file to VBIN
##
echo -n "[asm]  "
assemble -g program -o inline.vbin      inline.asm      && echo "ok" || echo "fail"

########################################################################################
##
## pack assets into the V32 ROM
##
echo -n "[pack] "
packrom                                 inline.xml      && echo "ok" || echo "fail"

exit 0
