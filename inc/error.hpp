#pragma once
////////////////////////////////////////////////////////////////////////////////////////
//
// error.hpp - the hardware error screen
//
////////////////////////////////////////////////////////////////////////////////////////

// where the error handler stashes the registers the hardware passes it:
// R0 error code, R1 address after the instruction at fault, R2 instruction, R3 immediate
#define  ERROR_SCRATCH           ADDR_BIOS_REGISTERS
#define  ERROR_ACTIVE_MARKER     0xE220E220

// draw the error screen for the values in ERROR_SCRATCH, and halt
void  report_error  (void);

// entry point of the hardware: the BIOS ROM starts with a call to this
void  error_handler (void);
