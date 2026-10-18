////////////////////////////////////////////////////////////////////////////////////////
//
// decode.cpp - machine code to assembly text
//
// Instruction word layout:
//
//   bits 31-26  opcode          bits 24-21  destination register
//   bit  25     immediate flag  bits 20-17  source register
//   bits 16-14  MOV address mode      bits 13-0   I/O port (IN / OUT)
//
// The mnemonics live in one string of fixed 6-character fields rather than in a
// 64-entry table of arrays: that table was built on the stack every time an
// instruction was decoded and was the heaviest single user of stack in the
// original debugger.
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
// decode_to_text()
//
void decode_to_text (Text *line, int instruction, int immediate)
{
    int  opcode    = (instruction >> 26) & 0x3F;
    int  immflag   = (instruction >> 25) & 1;
    int  dstreg    = (instruction >> 21) & 15;
    int  srcreg    = (instruction >> 17) & 15;
    int  mode      = (instruction >> 14) & 7;
    int  port      = instruction & INSTR_PORT_MASK;

    line->field
    (
        "HLT   WAIT  JMP   CALL  RET   JT    JF    IEQ   "
        "INE   IGT   IGE   ILT   ILE   FEQ   FNE   FGT   "
        "FGE   FLT   FLE   MOV   LEA   PUSH  POP   IN    "
        "OUT   MOVS  SETS  CMPS  CIF   CFI   CIB   CFB   "
        "NOT   AND   OR    XOR   BNOT  SHL   IADD  ISUB  "
        "IMUL  IDIV  IMOD  ISGN  IMIN  IMAX  IABS  FADD  "
        "FSUB  FMUL  FDIV  FMOD  FSGN  FMIN  FMAX  FABS  "
        "FLR   CEIL  ROUND SIN   ACOS  ATAN2 LOG   POW   ",
        opcode, 6
    );

    switch (opcode)
    {
        ////////////////////////////////////////////////////////////////////////////////
        //
        // no operands
        //
        case OPCODE_HLT:
        case OPCODE_WAIT:
        case OPCODE_RET:
        case OPCODE_MOVS:
        case OPCODE_SETS:
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // one operand: an immediate value, or the destination register
        //
        case OPCODE_JMP:
        case OPCODE_CALL:
        case OPCODE_PUSH:
        case OPCODE_POP:
        case OPCODE_CMPS:
        case OPCODE_CIF:
        case OPCODE_CFI:
        case OPCODE_CIB:
        case OPCODE_CFB:
        case OPCODE_NOT:
        case OPCODE_BNOT:
        case OPCODE_ISGN:
        case OPCODE_IABS:
        case OPCODE_FSGN:
        case OPCODE_FABS:
        case OPCODE_FLR:
        case OPCODE_CEIL:
        case OPCODE_ROUND:
        case OPCODE_SIN:
        case OPCODE_ACOS:
        case OPCODE_LOG:
            if (immflag == 1)
            {
                line->hex32 (immediate);
            }
            else
            {
                line->reg (dstreg);
            }
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // two operands, integer: destination register, then immediate or register
        //
        case OPCODE_JT:
        case OPCODE_JF:
        case OPCODE_IEQ:
        case OPCODE_INE:
        case OPCODE_IGT:
        case OPCODE_IGE:
        case OPCODE_ILT:
        case OPCODE_ILE:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        case OPCODE_SHL:
        case OPCODE_IADD:
        case OPCODE_ISUB:
        case OPCODE_IMUL:
        case OPCODE_IDIV:
        case OPCODE_IMOD:
        case OPCODE_IMIN:
        case OPCODE_IMAX:
            line->reg (dstreg);
            line->str (", ");
            if (immflag == 1)
            {
                line->hex32 (immediate);
            }
            else
            {
                line->reg (srcreg);
            }
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // two operands, float: an immediate is shown as the float it holds
        //
        case OPCODE_FEQ:
        case OPCODE_FNE:
        case OPCODE_FGT:
        case OPCODE_FGE:
        case OPCODE_FLT:
        case OPCODE_FLE:
        case OPCODE_FADD:
        case OPCODE_FSUB:
        case OPCODE_FMUL:
        case OPCODE_FDIV:
        case OPCODE_FMOD:
        case OPCODE_FMIN:
        case OPCODE_FMAX:
        case OPCODE_ATAN2:
        case OPCODE_POW:
            line->reg (dstreg);
            line->str (", ");
            if (immflag == 1)
            {
                line->flt (immediate);
            }
            else
            {
                line->reg (srcreg);
            }
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // MOV: the address mode says where the operands are
        //
        case OPCODE_MOV:
            switch (mode)
            {
                case 0:  // MOV Rd, imm
                    line->reg   (dstreg);
                    line->str   (", ");
                    line->hex32 (immediate);
                    break;

                case 1:  // MOV Rd, Rs
                    line->reg   (dstreg);
                    line->str   (", ");
                    line->reg   (srcreg);
                    break;

                case 2:  // MOV Rd, [imm]
                    line->reg   (dstreg);
                    line->str   (", [");
                    line->hex32 (immediate);
                    line->add   (']');
                    break;

                case 3:  // MOV Rd, [Rs]
                    line->reg   (dstreg);
                    line->str   (", [");
                    line->reg   (srcreg);
                    line->add   (']');
                    break;

                case 4:  // MOV Rd, [Rs+imm]
                    line->reg   (dstreg);
                    line->str   (", [");
                    line->reg   (srcreg);
                    line->add   ('+');
                    line->hex32 (immediate);
                    line->add   (']');
                    break;

                case 5:  // MOV [imm], Rs
                    line->add   ('[');
                    line->hex32 (immediate);
                    line->str   ("], ");
                    line->reg   (srcreg);
                    break;

                case 6:  // MOV [Rd], Rs
                    line->add   ('[');
                    line->reg   (dstreg);
                    line->str   ("], ");
                    line->reg   (srcreg);
                    break;

                case 7:  // MOV [Rd+imm], Rs
                    line->add   ('[');
                    line->reg   (dstreg);
                    line->add   ('+');
                    line->hex32 (immediate);
                    line->str   ("], ");
                    line->reg   (srcreg);
                    break;
            }
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // LEA Rd, [Rs] or LEA Rd, [Rs+imm]
        //
        case OPCODE_LEA:
            line->reg (dstreg);
            line->str (", [");
            line->reg (srcreg);
            if (immflag == 1)
            {
                line->add   ('+');
                line->hex32 (immediate);
            }
            line->add (']');
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // IN Rd, port
        //
        case OPCODE_IN:
            line->reg (dstreg);
            line->str (", 0x");
            line->hex (port, 3);
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // OUT port, immediate or register
        //
        case OPCODE_OUT:
            line->str ("0x");
            line->hex (port, 3);
            line->str (", ");
            if (immflag == 1)
            {
                line->hex32 (immediate);
            }
            else
            {
                line->reg (srcreg);
            }
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// decode(): decode an instruction, and draw it at the font's narrowed size
//
void decode (int x, int y, int instruction, int immediate)
{
    int   buffer[48];
    Text  line;

    line.init        (buffer);
    decode_to_text   (&line, instruction, immediate);
    ui_text          (x, y, line.text (), 1.0, 1.0);
}
