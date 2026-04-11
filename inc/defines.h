#ifndef _DEFINES_H
#define _DEFINES_H

////////////////////////////////////////////////////////////////////////////////////////
//
// Pre-processor: include indicated standard Vircon32 libraries
//
#include "audio.h"
#include "input.h"
#include "video.h"
#include "time.h"
#include "string.h"

////////////////////////////////////////////////////////////////////////////////////////
//
// Pre-processor: error codes
//
enum error_codes
{
    error_memory_read = 0,
    error_memory_write,
    error_port_read,
    error_port_write,
    error_stack_overflow,
    error_stack_underflow,
    error_division,
    error_arc_cosine,
    error_arc_tangent_2,
    error_logarithm,
    error_power,
    error_unknown
};

////////////////////////////////////////////////////////////////////////////////////////
//
// error_message struct: storing the name and description of each error
//
struct error_message
{
    int [50]  title;
    int [150] description;
};

////////////////////////////////////////////////////////////////////////////////////////
//
// Pre-processor: boolean
//
#define  TRUE          1
#define  FALSE         0

////////////////////////////////////////////////////////////////////////////////////////
//
// Pre-processor: opcode defines
//
#define  OPCODE_HLT    0x00
#define  OPCODE_WAIT   0x01
#define  OPCODE_JMP    0x02
#define  OPCODE_CALL   0x03
#define  OPCODE_RET    0x04
#define  OPCODE_JT     0x05
#define  OPCODE_JF     0x06
#define  OPCODE_IEQ    0x07
#define  OPCODE_INE    0x08
#define  OPCODE_IGT    0x09
#define  OPCODE_IGE    0x0A
#define  OPCODE_ILT    0x0B
#define  OPCODE_ILE    0x0C
#define  OPCODE_FEQ    0x0D
#define  OPCODE_FNE    0x0E
#define  OPCODE_FGT    0x0F
#define  OPCODE_FGE    0x10
#define  OPCODE_FLT    0x11
#define  OPCODE_FLE    0x12
#define  OPCODE_MOV    0x13
#define  OPCODE_LEA    0x14
#define  OPCODE_PUSH   0x15
#define  OPCODE_POP    0x16
#define  OPCODE_IN     0x17
#define  OPCODE_OUT    0x18
#define  OPCODE_MOVS   0x19
#define  OPCODE_SETS   0x1A
#define  OPCODE_CMPS   0x1B
#define  OPCODE_CIF    0x1C
#define  OPCODE_CFI    0x1D
#define  OPCODE_CIB    0x1E
#define  OPCODE_CFB    0x1F
#define  OPCODE_NOT    0x20
#define  OPCODE_AND    0x21
#define  OPCODE_OR     0x22
#define  OPCODE_XOR    0x23
#define  OPCODE_BNOT   0x24
#define  OPCODE_SHL    0x25
#define  OPCODE_IADD   0x26
#define  OPCODE_ISUB   0x27
#define  OPCODE_IMUL   0x28
#define  OPCODE_IDIV   0x29
#define  OPCODE_IMOD   0x2A
#define  OPCODE_ISGN   0x2B
#define  OPCODE_IMIN   0x2C
#define  OPCODE_IMAX   0x2D
#define  OPCODE_IABS   0x2E
#define  OPCODE_FADD   0x2F
#define  OPCODE_FSUB   0x30
#define  OPCODE_FMUL   0x31
#define  OPCODE_FDIV   0x32
#define  OPCODE_FMOD   0x33
#define  OPCODE_FSGN   0x34
#define  OPCODE_FMIN   0x35
#define  OPCODE_FMAX   0x36
#define  OPCODE_FABS   0x37
#define  OPCODE_FLR    0x38
#define  OPCODE_CEIL   0x39
#define  OPCODE_ROUND  0x3A
#define  OPCODE_SIN    0x3B
#define  OPCODE_ACOS   0x3C
#define  OPCODE_ATAN2  0x3D
#define  OPCODE_LOG    0x3E
#define  OPCODE_POW    0x3F

////////////////////////////////////////////////////////////////////////////////////////
//
// Pre-processor: memory map aliases
//
#define  ADDR_CART_REGISTERS 0x003FFBA0
#define  ADDR_CUSTOM_ROUTINE 0x003FFFB0
#define  ADDR_CART_OFFSET    0x003FFFEE
#define  ADDR_CUSTOM_RETURN  0x003FFFEF
#define  ADDR_BIOS_REGISTERS 0x003FFFF0

////////////////////////////////////////////////////////////////////////////////////////
//
// Pre-processor: gamepad button press easy read symbols
//
#define  BUTTON_NOT_PRESSED  -1
#define  BUTTON_IS_PRESSED   1

#define  DEBUG_ASM           0
#define  DEBUG_C             1

////////////////////////////////////////////////////////////////////////////////////////
//
// Pre-processor: continue mode settings
//
#define  CONTINUE_NONE      0
#define  CONTINUE_ENTRIGGER 1
#define  CONTINUE_ENABLED   2
#define  CONTINUE_DETRIGGER 3

////////////////////////////////////////////////////////////////////////////////////////
//
// Pre-processor: debugger view modes
//
#define  MODE_NONE     0
#define  MODE_REGISTER 1
#define  MODE_MEMORY   2
#define  MODE_STACK    3
#define  MODE_GPUPORTS 4
#define  MODE_SPUPORTS 5
#define  MODE_INPPORTS 6
#define  MAX_MODES     7

////////////////////////////////////////////////////////////////////////////////////////
//
// string_data: to simplify opcode display
//
struct string_data
{
    int [40] name;
    int      lower;
    int      upper;
};

////////////////////////////////////////////////////////////////////////////////////////
//
// opcode_t: The base unit of the array that stores the strings of each
// instruction opcode
//
struct opcode_t
{
    int [7] name;
};

////////////////////////////////////////////////////////////////////////////////////////
//
// BIOS-required regions; these
// may safely be used by programs
//
#define first_region_font      0
#define region_white_pixel   256

////////////////////////////////////////////////////////////////////////////////////////
//
// other non-required regions, used
// to draw the logo and error screens
//
#define region_logo_letters  300
#define region_logo_line     301
#define region_subtitle      302
#define region_console       303
#define region_gamepad       304
#define region_cartridge     305
#define region_memory_card   306
#define region_large_arrow   307
#define region_small_arrow   308
#define region_white_square  309
#define region_logo_v        310 // 1-58, 
#define region_logo_32       311 // 200+
#define region_divider_v     312
#define region_divider_h     313

////////////////////////////////////////////////////////////////////////////////////////
//
// an auxiliary region that will keep
// being redefined for logo animation
//
#define region_logo_temp     400

////////////////////////////////////////////////////////////////////////////////////////
//
// colors for error screens
//
#define error_colors_background  0xFF8D4130
#define error_colors_title       color_yellow
#define error_colors_message     color_white
#define error_colors_values      0xFF8080FF

////////////////////////////////////////////////////////////////////////////////////////
//
// hardware error codes
//
#define  ERROR_MEMORY_READ       0
#define  ERROR_MEMORY_WRITE      1
#define  ERROR_PORT_READ         2
#define  ERROR_PORT_WRITE        3
#define  ERROR_STACK_OVERFLOW    4
#define  ERROR_STACK_UNDERFLOW   5
#define  ERROR_DIVISION          6
#define  ERROR_ARC_COSINE        7
#define  ERROR_ARC_TANGENT_2     8
#define  ERROR_LOGARITHM         9
#define  ERROR_POWER            10
#define  ERROR_UNKNOWN          11

////////////////////////////////////////////////////////////////////////////////////////
//
// function prototypes
//
void  draw_message_screen (int *title, int *msg);
void  decode              (int  x,     int  y,   int  instruction, int    immediate);
void  hexit_zoomed        (int  x,     int  y,   int  value,       float  factor);
void  hexit               (int  x,     int  y,   int  value);
void  portit              (int  x,     int  y,   int  value);
void  print_hex_value     (int  x,     int  y,   int *name,        int    value);
void  print_zoomed_at     (int  x,     int  y,   int *text,        float  factor);
void  zprint_zoomed_at    (int  x,     int  y,   int *text,        float  factor);
void  init_regions        (void);
void  views               (int  mode,  int  mem, int  stack,       int    gamepad);
bool  cartridge_connected ();
void  draw_logo           (int *coffset);
void  request_cartridge   ();

#endif
