#pragma once
////////////////////////////////////////////////////////////////////////////////////////
//
// ports.hpp - raw I/O port access and the CART I/O shadow
//
////////////////////////////////////////////////////////////////////////////////////////

// read any I/O port by number (the CPU's IN instruction only takes a fixed port,
// so the instruction is assembled on the fly and called)
int   read_port        (int port);

// park the CART's GPU and gamepad-selection state in the shadow words of the
// memory map, so the BIOS is free to draw and to look at the gamepads
void  save_cart_io     (void);

// put the shadowed state back into the hardware, ready for the CART to run
void  restore_cart_io  (void);
