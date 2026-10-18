#pragma once
////////////////////////////////////////////////////////////////////////////////////////
//
// ui.hpp - drawing: text, numbers, the logo, the message screens
//
// All of it draws with the BIOS texture, and assumes the GPU belongs to the BIOS
// (ui_begin() was called since the CART last ran).
//
////////////////////////////////////////////////////////////////////////////////////////

// define the texture regions, and stop with a message if there is no cartridge
void  init_regions         (void);

// select the BIOS texture and reset the drawing state to known values
void  ui_begin             (void);

// draw text: x scale `sx`, y scale `sy` (1.0 = the font as it is). Line feeds
// move to the next line; tab and carriage return are not drawn.
void  ui_text              (int x, int y, int *text, float sx, float sy);

// the same at the font's own size
void  ui_print             (int x, int y, int *text);

// the same, narrowed by `factor` (the debugger's small text)
void  ui_small             (int x, int y, int *text, float factor);

// "0x" and 8 hex digits, narrowed by `factor` and stretched 2x in height (the
// debugger's addresses and values)
void  ui_hex               (int x, int y, int value, float factor);

// "0x" and 3 hex digits at the font's own size (I/O port numbers)
void  ui_port              (int x, int y, int value);

// header, shared by all the resource views: a title and a rule below it
void  ui_view_title        (int *title);

// the left, flickerless part of the screen: logo, controls help
void  draw_logo            (int mode, int keyboard);

// full-screen messages
void  draw_message_screen  (int *title, int *message);
void  request_cartridge    (void);
