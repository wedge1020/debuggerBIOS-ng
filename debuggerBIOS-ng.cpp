////////////////////////////////////////////////////////////////////////////////////////
//
// debuggerBIOS-ng.cpp - a Vircon32 BIOS with an interactive debugger
//
// Starts  in single-step  mode, waiting  for confirmation  on the  first
// CART  offset, 0x20000000.  DOWN single-steps,  UP cycles  the resource
// views,  LEFT/RIGHT and  L/R adjust  the content  of the  current view,
// START continues  (CONTINUATION mode:  the CART  runs, still  under the
// debugger's control) and X escapes  (leaves the debugger). START, X and
// DOWN act when  the button is released. A v32kbd  keyboard on the third
// gamepad port does the same (see README.md).
//
// Build with Make.sh (or Make.bat): the C++ is turned into Vircon32 C by
// v32c++.
//
////////////////////////////////////////////////////////////////////////////////////////

#title   "Vircon32 debuggerBIOS-ng"
#version 2.0
#texture BiosTexture "assets/BiosTexture.png"
#sound   BiosSound   "assets/BiosSound.wav"

////////////////////////////////////////////////////////////////////////////////////////
//
// the Vircon32 libraries
//
#include "audio.h"
#include "input.h"
#include "video.h"
#include "time.h"
#include "string.h"

////////////////////////////////////////////////////////////////////////////////////////
//
// declarations, in dependency order
//
#include "inc/config.hpp"
#include "inc/defines.hpp"
#include "inc/ring.hpp"
#include "inc/text.hpp"
#include "inc/ports.hpp"
#include "inc/ui.hpp"
#include "inc/decode.hpp"
#include "inc/input.hpp"
#include "inc/debugger.hpp"
#include "inc/error.hpp"

////////////////////////////////////////////////////////////////////////////////////////
//
// definitions
//
#include "src/ports.cpp"
#include "src/ui.cpp"
#include "src/decode.cpp"
#include "src/input.cpp"
#include "src/exec.cpp"
#include "src/views.cpp"
#include "src/debugger.cpp"
#include "src/error.cpp"

////////////////////////////////////////////////////////////////////////////////////////
//
// main(): normal BIOS routine run on system startup
//
void main (void)
{

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // everything  the  debugger  remembers   is  in   this  object  (see
    // debugger.hpp)
    //
    Debugger  debugger;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Move to the debugger's own stack. The CART will be given the stack
    // that  starts below  it, so  it gets  a stack  of its  own and  the
    // debugger can never be overwritten by what the CART pushes.
    //
    // main() never  returns, so  it does  not matter  that the  stack it
    // started on is left  behind. All it takes is to move  BP to the new
    // base and move SP by the  same amount, which keeps the frame main()
    // has just allocated  (for the local variables,  which are addressed
    // from BP) the same size.
    //
    asm
    {
        "PUSH R10"                       // back up R10 (arbitrary register)
        "MOV  R10,          R14"         // copy base pointer into R10
        "MOV  R14,          0x003FFF8F"  // new stack base
        "ISUB R10,          R14"         // how far the base moved
        "ISUB R15,          R10"         // move SP the same
        "POP  R10"                       // restore R10 (from the new stack)
    }

    init_regions ();
    debugger.init ();
    debugger.run ();

    asm { "hlt" }
}
