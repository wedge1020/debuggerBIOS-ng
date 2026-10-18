#pragma once
////////////////////////////////////////////////////////////////////////////////////////
//
// defines.hpp - constants shared by every part of debuggerBIOS-ng: opcodes, port
//               numbers, the debugger memory map, texture regions and colors.
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
// booleans
//
#define  TRUE                    1
#define  FALSE                   0

////////////////////////////////////////////////////////////////////////////////////////
//
// CPU opcodes (bits 31-26 of an instruction word)
//
#define  OPCODE_HLT              0x00
#define  OPCODE_WAIT             0x01
#define  OPCODE_JMP              0x02
#define  OPCODE_CALL             0x03
#define  OPCODE_RET              0x04
#define  OPCODE_JT               0x05
#define  OPCODE_JF               0x06
#define  OPCODE_IEQ              0x07
#define  OPCODE_INE              0x08
#define  OPCODE_IGT              0x09
#define  OPCODE_IGE              0x0A
#define  OPCODE_ILT              0x0B
#define  OPCODE_ILE              0x0C
#define  OPCODE_FEQ              0x0D
#define  OPCODE_FNE              0x0E
#define  OPCODE_FGT              0x0F
#define  OPCODE_FGE              0x10
#define  OPCODE_FLT              0x11
#define  OPCODE_FLE              0x12
#define  OPCODE_MOV              0x13
#define  OPCODE_LEA              0x14
#define  OPCODE_PUSH             0x15
#define  OPCODE_POP              0x16
#define  OPCODE_IN               0x17
#define  OPCODE_OUT              0x18
#define  OPCODE_MOVS             0x19
#define  OPCODE_SETS             0x1A
#define  OPCODE_CMPS             0x1B
#define  OPCODE_CIF              0x1C
#define  OPCODE_CFI              0x1D
#define  OPCODE_CIB              0x1E
#define  OPCODE_CFB              0x1F
#define  OPCODE_NOT              0x20
#define  OPCODE_AND              0x21
#define  OPCODE_OR               0x22
#define  OPCODE_XOR              0x23
#define  OPCODE_BNOT             0x24
#define  OPCODE_SHL              0x25
#define  OPCODE_IADD             0x26
#define  OPCODE_ISUB             0x27
#define  OPCODE_IMUL             0x28
#define  OPCODE_IDIV             0x29
#define  OPCODE_IMOD             0x2A
#define  OPCODE_ISGN             0x2B
#define  OPCODE_IMIN             0x2C
#define  OPCODE_IMAX             0x2D
#define  OPCODE_IABS             0x2E
#define  OPCODE_FADD             0x2F
#define  OPCODE_FSUB             0x30
#define  OPCODE_FMUL             0x31
#define  OPCODE_FDIV             0x32
#define  OPCODE_FMOD             0x33
#define  OPCODE_FSGN             0x34
#define  OPCODE_FMIN             0x35
#define  OPCODE_FMAX             0x36
#define  OPCODE_FABS             0x37
#define  OPCODE_FLR              0x38
#define  OPCODE_CEIL             0x39
#define  OPCODE_ROUND            0x3A
#define  OPCODE_SIN              0x3B
#define  OPCODE_ACOS             0x3C
#define  OPCODE_ATAN2            0x3D
#define  OPCODE_LOG              0x3E
#define  OPCODE_POW              0x3F

////////////////////////////////////////////////////////////////////////////////////////
//
// instruction word fields
//
#define  INSTR_IMMEDIATE_BIT     0x02000000   // bit 25
#define  INSTR_PORT_MASK         0x00003FFF   // bits 13-0

////////////////////////////////////////////////////////////////////////////////////////
//
// opcodes that the executor must treat specially (bit n set = opcode n). Every
// other opcode is "plain": it is copied as it is into the native routine.
//
//   HLT WAIT JMP CALL RET JT JF IN OUT
//
#define  SPECIAL_OPCODES         ((1 << OPCODE_HLT)  | (1 << OPCODE_WAIT) | \
                                  (1 << OPCODE_JMP)  | (1 << OPCODE_CALL) | \
                                  (1 << OPCODE_RET)  | (1 << OPCODE_JT)   | \
                                  (1 << OPCODE_JF)   | (1 << OPCODE_IN)   | \
                                  (1 << OPCODE_OUT))

////////////////////////////////////////////////////////////////////////////////////////
//
// I/O port numbers (device base + register number)
//
#define  PORT_TIM_DATE           0x000
#define  PORT_TIM_TIME           0x001
#define  PORT_TIM_FRAMECOUNTER   0x002
#define  PORT_TIM_CYCLECOUNTER   0x003
#define  PORT_RNG_VALUE          0x100
#define  PORT_GPU_COMMAND        0x200
#define  PORT_GPU_REMAINING      0x201
#define  PORT_GPU_CLEARCOLOR     0x202
#define  PORT_GPU_MULTIPLY       0x203
#define  PORT_GPU_BLEND          0x204
#define  PORT_GPU_TEXTURE        0x205
#define  PORT_GPU_REGION         0x206
#define  PORT_GPU_DRAWX          0x207
#define  PORT_GPU_DRAWY          0x208
#define  PORT_GPU_SCALEX         0x209
#define  PORT_GPU_SCALEY         0x20A
#define  PORT_GPU_ANGLE          0x20B
#define  PORT_GPU_REGION_MINX    0x20C
#define  PORT_GPU_REGION_MINY    0x20D
#define  PORT_GPU_REGION_MAXX    0x20E
#define  PORT_GPU_REGION_MAXY    0x20F
#define  PORT_GPU_REGION_HOTX    0x210
#define  PORT_GPU_REGION_HOTY    0x211
#define  PORT_SPU_COMMAND        0x300
#define  PORT_SPU_VOLUME         0x301
#define  PORT_SPU_SOUND          0x302
#define  PORT_SPU_CHANNEL        0x303
#define  PORT_SPU_SOUND_LENGTH   0x304
#define  PORT_SPU_SOUND_LOOP     0x305
#define  PORT_SPU_LOOP_START     0x306
#define  PORT_SPU_LOOP_END       0x307
#define  PORT_SPU_CHAN_STATE     0x308
#define  PORT_SPU_CHAN_SOUND     0x309
#define  PORT_SPU_CHAN_VOLUME    0x30A
#define  PORT_SPU_CHAN_SPEED     0x30B
#define  PORT_SPU_CHAN_LOOP      0x30C
#define  PORT_SPU_CHAN_POSITION  0x30D
#define  PORT_INP_SELECTED       0x400
#define  PORT_INP_CONNECTED      0x401
#define  PORT_INP_LEFT           0x402
#define  PORT_INP_RIGHT          0x403
#define  PORT_INP_UP             0x404
#define  PORT_INP_DOWN           0x405
#define  PORT_INP_START          0x406
#define  PORT_INP_A              0x407
#define  PORT_INP_B              0x408
#define  PORT_INP_X              0x409
#define  PORT_INP_Y              0x40A
#define  PORT_INP_L              0x40B
#define  PORT_INP_R              0x40C
#define  PORT_CAR_CONNECTED      0x500
#define  PORT_CAR_ROMSIZE        0x501
#define  PORT_CAR_TEXTURES       0x502
#define  PORT_CAR_SOUNDS         0x503
#define  PORT_MEM_CONNECTED      0x600

////////////////////////////////////////////////////////////////////////////////////////
//
// GPU command values
//
#define  GPUCMD_CLEAR            0x10

////////////////////////////////////////////////////////////////////////////////////////
//
// address ranges
//
#define  ADDR_CART_START         0x20000000
#define  ADDR_MEMCARD            0x30000000
#define  ADDR_MEMCARD_LAST       0x3003FFF0   // last 16-word row of the card
#define  ADDR_RAM_LAST_ROW       0x003FFFF0   // last 16-word row of RAM

////////////////////////////////////////////////////////////////////////////////////////
//
// debuggerBIOS-ng memory map. RAM is shared with the CART, so the debugger keeps
// everything it owns at the very top, where a CART is least likely to go. The
// CART gets its own stack right below the reserved area.
//
//   0x003FFFF0-0x003FFFFF  BIOS R0-R15, saved while the CART instruction runs
//   0x003FFFEF             where the native routine returns to
//   0x003FFFEE             CART address of the first instruction of the block
//                          being executed (0xDEADBEEF after ESCAPE)
//   0x003FFFED             CART TIM_FrameCounter (as the CART should see it)
//   0x003FFFEC             CART TIM_CycleCounter
//   0x003FFFEB             CART RNG_CurrentValue
//   0x003FFFEA             CART INP_SelectedGamepad       \
//   0x003FFFE9             CART GPU_DrawingAngle           |
//   0x003FFFE8             CART GPU_DrawingScaleY          |
//   0x003FFFE7             CART GPU_DrawingScaleX          |
//   0x003FFFE6             CART GPU_DrawingPointY          | CART I/O port shadow,
//   0x003FFFE5             CART GPU_DrawingPointX          | valid while the BIOS
//   0x003FFFE4             CART GPU_SelectedRegion         | owns the hardware
//   0x003FFFE3             CART GPU_SelectedTexture        |
//   0x003FFFE2             CART GPU_ActiveBlending         |
//   0x003FFFE1             CART GPU_MultiplyColor          |
//   0x003FFFE0             CART GPU_ClearColor            /
//   0x003FFFD0-0x003FFFDF  CART R0-R15
//   0x003FFF90-0x003FFFCF  native routine (copy of the CART instructions)
//   0x003FFC90-0x003FFF8F  debugger stack (768 words)
//   0x003FFC8F             base of the CART stack (initial CART BP/SP)
//
#define  ADDR_BIOS_REGISTERS     0x003FFFF0
#define  ADDR_CUSTOM_RETURN      0x003FFFEF
#define  ADDR_CART_OFFSET        0x003FFFEE
#define  ADDR_PORT_TIMFRAME      0x003FFFED
#define  ADDR_PORT_TIMCYCLE      0x003FFFEC
#define  ADDR_PORT_RNGVALUE      0x003FFFEB
#define  ADDR_SHADOW             0x003FFFE0   // first of the 11 shadow words
#define  SHADOW_WORDS            11
#define  SHADOW_CLEAR            0
#define  SHADOW_MULTIPLY         1
#define  SHADOW_BLEND            2
#define  SHADOW_TEXTURE          3
#define  SHADOW_REGION           4
#define  SHADOW_DRAWX            5
#define  SHADOW_DRAWY            6
#define  SHADOW_SCALEX           7
#define  SHADOW_SCALEY           8
#define  SHADOW_ANGLE            9
#define  SHADOW_GAMEPAD          10
#define  ADDR_CART_REGISTERS     0x003FFFD0
#define  ADDR_CUSTOM_ROUTINE     0x003FFF90
#define  ROUTINE_WORDS           64
#define  ADDR_DEBUGGER_BASE      0x003FFF8F   // BP of the debugger stack
#define  ADDR_CART_STACK_BASE    0x003FFC8F
#define  ESCAPED_MARKER          0xDEADBEEF

////////////////////////////////////////////////////////////////////////////////////////
//
// instruction words that go into the native routine after the CART code
//
#define  WORD_PUSH_R10           0x55400000   // PUSH R10
#define  WORD_MOV_R10_MEM        0x4F408000   // MOV R10, [immediate]
#define  WORD_JMP_R10            0x09400000   // JMP R10
#define  WORD_HLT                0x00000000   // HLT
#define  WORD_JMP_IMMEDIATE      0x0A000000   // JMP immediate

////////////////////////////////////////////////////////////////////////////////////////
//
// gamepad values: pressed is anything above 0 (frames held), released below 0
//
#define  BUTTON_IS_PRESSED       1

////////////////////////////////////////////////////////////////////////////////////////
//
// debugger controls (one bit each). The same bits describe a gamepad button
// and the keyboard key that stands in for it.
//
#define  CTRL_UP                 0x01
#define  CTRL_DOWN               0x02
#define  CTRL_LEFT               0x04
#define  CTRL_RIGHT              0x08
#define  CTRL_START              0x10
#define  CTRL_X                  0x20
#define  CTRL_L                  0x40
#define  CTRL_R                  0x80
#define  NUM_CONTROLS            8

////////////////////////////////////////////////////////////////////////////////////////
//
// actions that fire when a control is released
//
#define  ACTION_NONE             0
#define  ACTION_STEP             1   // DOWN
#define  ACTION_CONTINUE         2   // START
#define  ACTION_ESCAPE           3   // X

////////////////////////////////////////////////////////////////////////////////////////
//
// debugger view modes
//
#define  MODE_NONE               0
#define  MODE_REG                1
#define  MODE_RAM                2
#define  MODE_STA                3
#define  MODE_BTR                4
#define  MODE_TIM                5
#define  MODE_RNG                6
#define  MODE_GPU                7
#define  MODE_SPU                8
#define  MODE_INP                9
#define  MODE_CAR                10
#define  MODE_MEM                11
#define  NUM_MODES               12

////////////////////////////////////////////////////////////////////////////////////////
//
// register view number formats
//
#define  FORMAT_HEX              0
#define  FORMAT_INTEGER          1
#define  FORMAT_FLOAT            2
#define  NUM_FORMATS             3

////////////////////////////////////////////////////////////////////////////////////////
//
// texture regions. The font occupies regions 0-255 (one per character code).
// The regions from 256 up are the BIOS-required ones plus those the logo and
// error screens use.
//
#define  first_region_font       0
#define  region_white_pixel      256
#define  region_logo_letters     300
#define  region_logo_line        301
#define  region_subtitle         302
#define  region_console          303
#define  region_gamepad          304
#define  region_cartridge        305
#define  region_memory_card      306
#define  region_large_arrow      307
#define  region_small_arrow      308
#define  region_white_square     309
#define  region_logo_v           310
#define  region_logo_32          311
#define  region_divider_v        312
#define  region_divider_h        313
#define  region_logo_temp        400

////////////////////////////////////////////////////////////////////////////////////////
//
// colors for the error screen
//
#define  error_colors_background 0xFF8D4130
#define  error_colors_title      color_yellow
#define  error_colors_message    color_white
#define  error_colors_values     0xFF8080FF

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
#define  ERROR_POWER             10
#define  ERROR_UNKNOWN           11
