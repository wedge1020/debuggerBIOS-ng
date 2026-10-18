#pragma once
////////////////////////////////////////////////////////////////////////////////////////
//
// config.hpp - user-tunable settings for debuggerBIOS
//
// Everything in here is a plain pre-processor value; change it and rebuild.
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
// Which gamepad port drives the debugger (0 = Gamepad 1 ... 3 = Gamepad 4)
//
#define  DEBUG_GAMEPAD            0

////////////////////////////////////////////////////////////////////////////////////////
//
// v32kbd support: a v32io keyboard on this gamepad port (0-3) can drive the
// debugger as well as the gamepad. The default is the third port ("Gamepad 3",
// port id 2). Set KEYBOARD_ENABLED to 0 to build without it.
//
// NOTE: only a v32io keyboard (v32kbd emulator device, or the v32io hardware
// adaptor) belongs on this port. An ordinary gamepad plugged in there would
// be read as a stream of nonsense key events.
//
#define  KEYBOARD_ENABLED         1
#define  KEYBOARD_GAMEPAD         2

////////////////////////////////////////////////////////////////////////////////////////
//
// Instruction history shown in the single-step display (power of two). One
// entry is the instruction about to run, so the screen shows this many minus
// one past instructions.
//
#define  HISTORY_SIZE             8

////////////////////////////////////////////////////////////////////////////////////////
//
// Subroutine backtrace depth that is remembered (power of two). Deeper call
// chains are still tracked correctly: the oldest frames simply drop out of the
// ring.
//
#define  TRACE_SIZE               64

////////////////////////////////////////////////////////////////////////////////////////
//
// CONTINUATION mode: the most CART instructions run in one go before the
// debugger looks at the controls again. They are copied together into the native
// routine and run with a single switch of context; each branch, WAIT or screen
// clear ends the run early. 1 would execute one instruction per switch, as the
// first debuggerBIOS did. At most 29 (the routine has 64 words; see defines.hpp).
//
#define  BLOCK_MAX                24

////////////////////////////////////////////////////////////////////////////////////////
//
// Navigation controls LEFT, RIGHT, L and R repeat while held: after this many
// frames, then every so many frames.
//
#define  NAV_REPEAT_DELAY         20
#define  NAV_REPEAT_RATE          3
