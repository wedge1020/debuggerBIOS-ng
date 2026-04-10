#!/usr/bin/env bash
##
## inject-debug.sh - script to process  the compiler and assembler .debug
## files to produce a  linkage from line of C source  to line of assembly
## to memory offset
##
##            usage: inject-debug.sh FILE.VBIN
##
## It will then proceed to inject that data (offsets and their lines of C
## code) into  an unsuspecting VBIN file,  for the purposes of  having an
## ability to single-step C code within debuggerBIOS-ng.
##
## This script  should be incorporated  in a Vircon32  cartridge building
## process, immediately after assembling and before packing the ROM.
##
##############################################################################

##############################################################################
##
## Declare and initialize variables
##
VBINFILE="${1}"
NAME=$(echo "${VBINFILE}" | cut -d'.' -f1)

##############################################################################
##
## Process the .asm.debug file, containing the C to assembly linkages
##
cat obj/${NAME}.asm.debug  | sed 's/^\([^,]*\),\([1-9][0-9]*\),\([^,]*\),\([1-9][0-9]*\).*$/\2:\1:\4:\3/g'  > 1

##############################################################################
##
## Process the .vbin.debug file, containing the assembly to machine 
## code linkages
##
cat obj/${VBINFILE}.debug | sed 's/^\(0x[0-9A-F][0-9A-F]*\),[^,]*,\([1-9][0-9]*\),\?.*$/\2:\1/g' > 2

##############################################################################
##
## Initialize the output file
##
echo -n  > out

count=0
for entry in `cat 1`; do
    asmline=$(echo "${entry}" | cut -d':' -f1)
    asmfile=$(echo "${entry}" | cut -d':' -f2)
    cline=$(echo   "${entry}" | cut -d':' -f3)
    cfile=$(echo   "${entry}" | cut -d':' -f4)

    chk=$(cat ${asmfile} | head -${asmline} | tail -1 | grep '^ *_[^:]*:$' | wc -l)

    while [ "${chk}" -eq 1 ]; do
        let asmline=asmline+1
        echo -n "[${asmline}] "
        cat ${asmfile} | head -${asmline} | tail -1
        chk=$(cat ${asmfile} | head -${asmline} | tail -1 | grep '^ *_[^:]*:$' | wc -l)
    done

    offset=$(cat 2 | grep "^${asmline}:" | cut -d':' -f2)
    ochk=$(echo "${offset}" | egrep -qio '\<0x[0-9A-F]{8}\>' && echo "true" || echo "false")
    if [ ! -z "${offset}" ] && [ "${ochk}" = "true" ]; then
        cdata=$(cat ${cfile} | head -${cline} | tail -1 | tr '\t' ' ' | tr -s ' ' | sed 's/^  *//g' | sed 's://.*$::g' | sed 's/  *$//g')
        b64data=$(echo -n "${cdata}" | base64 -w 0)
        if [ ! -z "${b64data}" ]; then
            echo "${offset}:${entry}:${b64data}" >> out
            echo "${offset}:${entry}:${b64data}"
        fi
    else
        echo "offset:  ${offset}"
        echo "entry:   ${entry}"
        echo "asmline: ${asmline}"
        echo "asmfile: ${asmfile}"
        echo "cline:   ${cline}"
        echo "cfile:   ${cfile}"
        echo "b64data: ${b64data}"
        echo "decoded: `echo ${b64data} | base64 -d`"
        exit 1
    fi
    let count=count+1
done
# for line in `cat 1 | cut -d':' -f1`; do offset=$(cat 2 | grep "${line}" | cut -d':' -f2); echo "${line}:${offset}" >> out; done

exit 0
