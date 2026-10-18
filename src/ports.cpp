////////////////////////////////////////////////////////////////////////////////////////
//
// ports.cpp - raw I/O port access and the CART I/O shadow
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
// read_port(): the instruction `IN R1, port` followed by a RET is written into a
// two-word buffer on the stack, and called. R1 is saved around it.
//
//   0x5C200000 = IN opcode (0x17 << 26) | destination R1 (1 << 21)
//   0x10000000 = RET
//
int read_port (int port)
{
    int  routine[2];
    int  entry          = 0;
    int  result         = 0;

    routine[0]          = 0x5C200000 | port;
    routine[1]          = 0x10000000;
    entry               = (int) &routine[0];

    asm
    {
        "PUSH R1"
        "MOV  R1,       {entry}"
        "CALL R1"
        "MOV  {result}, R1"
        "POP  R1"
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// save_cart_io(): while the CART runs, the GPU ports and the selected gamepad
// are the CART's. The first time the BIOS needs to draw they are copied into the
// shadow words (see the memory map in defines.hpp).
//
// The selected gamepad is NOT among the things read: reading INP_SelectedGamepad
// does not return the selection on the console (it returns the previous word of
// the gamepad state), so the shadow word for it is kept up to date by the
// debugger, which sees every OUT the CART does to that port.
//
void save_cart_io (void)
{
    asm
    {
        "PUSH R0"
        "IN   R0,           GPU_ClearColor"
        "MOV  [0x003FFFE0], R0"
        "IN   R0,           GPU_MultiplyColor"
        "MOV  [0x003FFFE1], R0"
        "IN   R0,           GPU_ActiveBlending"
        "MOV  [0x003FFFE2], R0"
        "IN   R0,           GPU_SelectedTexture"
        "MOV  [0x003FFFE3], R0"
        "IN   R0,           GPU_SelectedRegion"
        "MOV  [0x003FFFE4], R0"
        "IN   R0,           GPU_DrawingPointX"
        "MOV  [0x003FFFE5], R0"
        "IN   R0,           GPU_DrawingPointY"
        "MOV  [0x003FFFE6], R0"
        "IN   R0,           GPU_DrawingScaleX"
        "MOV  [0x003FFFE7], R0"
        "IN   R0,           GPU_DrawingScaleY"
        "MOV  [0x003FFFE8], R0"
        "IN   R0,           GPU_DrawingAngle"
        "MOV  [0x003FFFE9], R0"
        "POP  R0"
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// restore_cart_io(): the opposite. The texture is selected before the region,
// because a region number only means something within a texture.
//
void restore_cart_io (void)
{
    asm
    {
        "PUSH R0"
        "MOV  R0,                  [0x003FFFE0]"
        "OUT  GPU_ClearColor,      R0"
        "MOV  R0,                  [0x003FFFE1]"
        "OUT  GPU_MultiplyColor,   R0"
        "MOV  R0,                  [0x003FFFE2]"
        "OUT  GPU_ActiveBlending,  R0"
        "MOV  R0,                  [0x003FFFE3]"
        "OUT  GPU_SelectedTexture, R0"
        "MOV  R0,                  [0x003FFFE4]"
        "OUT  GPU_SelectedRegion,  R0"
        "MOV  R0,                  [0x003FFFE5]"
        "OUT  GPU_DrawingPointX,   R0"
        "MOV  R0,                  [0x003FFFE6]"
        "OUT  GPU_DrawingPointY,   R0"
        "MOV  R0,                  [0x003FFFE7]"
        "OUT  GPU_DrawingScaleX,   R0"
        "MOV  R0,                  [0x003FFFE8]"
        "OUT  GPU_DrawingScaleY,   R0"
        "MOV  R0,                  [0x003FFFE9]"
        "OUT  GPU_DrawingAngle,    R0"
        "MOV  R0,                  [0x003FFFEA]"
        "OUT  INP_SelectedGamepad, R0"
        "POP  R0"
    }
}
