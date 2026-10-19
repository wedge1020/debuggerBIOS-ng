# debuggerBIOS-ng (C++ edition)

Vircon32 BIOS with an interactive debugger, written in C++ for the v32cxx transpiler.
The CART is never run directly: its instructions are copied into a RAM routine and run
under control, so you can step, continue or escape at any point.

## Build
Put the Vircon32 DevTools and `v32c++` in the PATH, then `./Make.sh` (or `Make.bat`).
If the DevTools headers are not beside `compile`, set `V32CXX_SDK_INCLUDE`.
Output: `bin/debuggerBIOS-ng.v32`.
Old v32c++ mangles `error_handler` even with `-b`; `Make.sh` repairs it, and
`patches/v32cxx-bios-error-handler.patch` fixes v32c++ itself.

## Layout
`debuggerBIOS-ng.cpp` (unity build) · `inc/` headers (config, defines, ring, text, ports, ui, decode,
input, debugger, error) · `src/` (ports, ui, decode, input, exec, views, debugger, error) ·
`assets/` · `docs/` · `tests/` · `tools/`. Tunables are in `inc/config.hpp`.

## Controls (act on RELEASE for DOWN / X / START+X)
| control | keyboard (v32kbd, port 3) | action |
|---|---|---|
| DOWN | Down, Space | step one instruction |
| START+X | Enter+x, c+Esc | start / stop CONTINUATION (fires when the combo is released) |
| X (alone) | Esc, x | ESCAPE: leave the debugger, CART runs natively |
| START (alone) | Enter, c | nothing (only part of the combo) |
| UP | Up | next content view |
| LEFT/RIGHT | Left/Right | small adjust (auto-repeat) |
| L/R | [ , / ] . | big adjust (auto-repeat) |

A press only counts if the control went down alone and nothing else went down before it was
released (START and X together form the one combo; either may go down first); otherwise the whole chord is ignored until everything is let go. Controls held at
power-on are ignored until released. The keyboard is optional (`KEYBOARD_ENABLED`,
`KEYBOARD_GAMEPAD` in config.hpp); a small built-in decoder is used, not the v32::Keyboard class.

## Performance
* History and call backtrace are ring buffers (no array shifting).
* CONTINUATION runs blocks of up to `BLOCK_MAX` (24) instructions per context switch; only
  branches, HLT, WAIT and some port reads are emulated.
* GPU/gamepad state is saved only when the BIOS draws (lazy shadowing).
* Measured on a counter loop: about 2.3x the iterations of the original debuggerBIOS-ng in the same time.
* Pathological case: unbounded recursion in CONTINUATION reaches the stack overflow error very
  slowly (it must exhaust ~4M stack words through emulated CALLs); after ESCAPE it is immediate.

## Notes
* The selected gamepad cannot be read back: on the console, reading INP_SelectedGamepad returns
  the previous word of the gamepad state, not the selection. The debugger therefore never reads it;
  it tracks the CART's selection itself (starting at gamepad 0, updated by every CART `OUT INP_SelectedGamepad`)
  and puts exactly that back on ESCAPE and whenever the CART resumes.
* BIOS has no safe globals (they collide with the CART at RAM 0): state lives in a `Debugger`
  object on the relocated stack; fixed words at the top of RAM are listed in `inc/defines.hpp`.
* Errors inside the RAM routine are mapped back to the CART address exactly.
* After ESCAPE the TIM frame counter jumps (it is emulated while the debugger runs).
* Hex values are drawn as the original did: "0x" scaled, digits at normal height with tightened spacing.
* See `docs/AUDIT.md` and `docs/TEXTURE.md`.
