////////////////////////////////////////////////////////////////////////////////////////
//
// decode(): take machine code instruction w/ immediate data and decode
// into assembly instruction
//
void decode (int x, int y, int instruction, int immediate)
{
    int [18] data;
    int      opcode   = (instruction & 0xFC000000) >> 26; // bits 31-26
    int      immflag  = (instruction & 0x02000000) >> 25; // bit  25
    int      dstreg   = (instruction & 0x01E00000) >> 21; // bits 24-21
    int      srcreg   = (instruction & 0x001E0000) >> 17; // bits 20-17
    int      addr     = (instruction & 0x0001C000) >> 14; // bits 16-14
    int      port     = (instruction & 0x00003FFF);       // bits 13-0

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // initialize our opcodes array with the available instructions
    //
    opcode_t [64] mneumonic =
    {
        { "HLT"  }, { "WAIT"  }, { "JMP"   }, { "CALL" },
        { "RET"  }, { "JT"    }, { "JF"    }, { "IEQ"  },
        { "INE"  }, { "IGT"   }, { "IGE"   }, { "ILT"  },
        { "ILE"  }, { "FEQ"   }, { "FNE"   }, { "FGT"  },
        { "FGE"  }, { "FLT"   }, { "FLE"   }, { "MOV"  },
        { "LEA"  }, { "PUSH"  }, { "POP"   }, { "IN"   },
        { "OUT"  }, { "MOVS"  }, { "SETS"  }, { "CMPS" },
        { "CIF"  }, { "CFI"   }, { "CIB"   }, { "CFB"  },
        { "NOT"  }, { "AND"   }, { "OR"    }, { "XOR"  },
        { "BNOT" }, { "SHL"   }, { "IADD"  }, { "ISUB" },
        { "IMUL" }, { "IDIV"  }, { "IMOD"  }, { "ISGN" },
        { "IMIN" }, { "IMAX"  }, { "IABS"  }, { "FADD" },
        { "FSUB" }, { "FMUL"  }, { "FDIV"  }, { "FMOD" },
        { "FSGN" }, { "FMIN"  }, { "FMAX"  }, { "FABS" },
        { "FLR"  }, { "CEIL"  }, { "ROUND" }, { "SIN"  },
        { "ACOS" }, { "ATAN2" }, { "LOG"   }, { "POW"  }
    };

    strcpy (data, mneumonic[opcode].name);
    print_at (x, y, data);
    x = x + 60;

    switch (opcode)
    {
        case OPCODE_HLT:  // zero operand instructions
        case OPCODE_WAIT:
        case OPCODE_RET:
        case OPCODE_MOVS:
        case OPCODE_SETS:
            break;

        case OPCODE_JMP:   // one operand instructions
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
                hexit (x, y, immediate);
            }
            else
            {
                strcpy (data, "R");
                print_at (x, y, data);
                x  = x + 10;
                itoa (dstreg, data, 10);
                print_at (x, y, data);
            }
            break;

        case OPCODE_JT: // two-operand integer instructions
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
            strcpy (data, "R");
            print_at (x, y, data);
            x  = x + 10;
            itoa (dstreg, data, 10);
            print_at (x, y, data);

            if (dstreg  >  9) // calibrate for register name length (R# vs R##)
            {
                x  = x + 10;
            }
            x      = x + 10;

            strcpy (data, ", ");
            print_at (x, y, data);
            x      = x + 20;

            if (immflag == 1)
            {
                hexit (x, y, immediate);
            }
            else
            {
                strcpy (data, "R");
                print_at (x, y, data);
                x  = x + 10;
                itoa (srcreg, data, 10);
                print_at (x, y, data);
            }
            break;

        case OPCODE_FEQ: // two-operand float instructions
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
            strcpy (data, "R");
            print_at (x, y, data);
            x  = x + 10;
            itoa (dstreg, data, 10);
            print_at (x, y, data);

            if (dstreg  >  9) // calibrate for register name length (R# vs R##)
            {
                x  = x + 10;
            }
            x      = x + 10;

            strcpy (data, ", ");
            print_at (x, y, data);

            if (dstreg  <= 9)
            {
                x  = x + 10;
            }
            x      = x + 150;

            if (immflag == 1)
            {
                ftoa (immediate, data);
            }
            else
            {
                strcpy (data, "R");
                print_at (x, y, data);
                x  = x + 10;
                itoa (srcreg, data, 10);
            }
            print_at (x, y, data);
            break;

        case OPCODE_MOV:
            switch (addr)
            {
                case 00:
                    strcpy (data, "R");
                    print_at (x, y, data);
                    x  = x + 10;
                    itoa (dstreg, data, 10);
                    print_at (x, y, data);
                    if (dstreg >  9)
                    {
                        x = x + 10;
                    }
                    x = x + 10;
                    strcpy (data, ",");
                    print_at (x, y, data);
                    x = x + 20;
                    hexit (x, y, immediate);
                    break;

                case 01:
                    strcpy (data, "R");
                    print_at (x, y, data);
                    x  = x + 10;
                    itoa (dstreg, data, 10);
                    print_at (x, y, data);
                    if (dstreg >  9)
                    {
                        x = x + 10;
                    }
                    x = x + 10;
                    strcpy (data, ", R");
                    print_at (x, y, data);
                    x = x + 30;
                    itoa (srcreg, data, 10);
                    print_at (x, y, data);
                    break;

                case 02:
                    strcpy (data, "R");
                    print_at (x, y, data);
                    x  = x + 10;
                    itoa (dstreg, data, 10);
                    print_at (x, y, data);
                    if (dstreg >  9)
                    {
                        x = x + 10;
                    }
                    x = x + 10;
                    strcpy (data, ", [");
                    print_at (x, y, data);
                    x = x + 30;
                    hexit (x, y, immediate);
                    x = x + 100;
                    strcpy (data, "]");
                    print_at (x, y, data);
                    break;

                case 03:
                    strcpy (data, "R");
                    print_at (x, y, data);
                    x  = x + 10;
                    itoa (dstreg, data, 10);
                    print_at (x, y, data);
                    if (dstreg >  9)
                    {
                        x = x + 10;
                    }
                    x = x + 10;
                    strcpy (data, ", [R");
                    print_at (x, y, data);
                    x = x + 40;
                    itoa (srcreg, data, 10);
                    print_at (x, y, data);
                    if (srcreg >  9)
                    {
                        x = x + 10;
                    }
                    x = x + 10;
                    strcpy (data, "]");
                    print_at (x, y, data);
                    break;

                case 04:
                    strcpy (data, "R");
                    print_at (x, y, data);
                    x  = x + 10;
                    itoa (dstreg, data, 10);
                    print_at (x, y, data);
                    if (dstreg >  9)
                    {
                        x = x + 10;
                    }
                    x = x + 10;
                    strcpy (data, ", [R");
                    print_at (x, y, data);
                    x = x + 40;
                    itoa (srcreg, data, 10);
                    print_at (x, y, data);
                    if (srcreg >  9)
                    {
                        x = x + 10;
                    }
                    x = x + 10;
                    strcpy (data, "+");
                    print_at (x, y, data);
                    x = x + 10;
                    hexit (x, y, immediate);
                    x = x + 100;
                    strcpy (data, "]");
                    print_at (x, y, data);
                    break;

                case 05:
                    strcpy (data, "[");
                    print_at (x, y, data);
                    x = x + 10;
                    hexit (x, y, immediate);
                    x = x + 100;
                    strcpy (data, "],");
                    print_at (x, y, data);
                    x = x + 30;
                    strcpy (data, "R");
                    print_at (x, y, data);
                    x  = x + 10;
                    itoa (srcreg, data, 10);
                    print_at (x, y, data);
                    break;

                case 06:
                    strcpy (data, "[R");
                    print_at (x, y, data);
                    x = x + 20;
                    itoa (dstreg, data, 10);
                    print_at (x, y, data);
                    if (dstreg >  9)
                    {
                        x = x + 10;
                    }
                    x = x + 10;
                    strcpy (data, "],");
                    print_at (x, y, data);
                    x = x + 30;
                    strcpy (data, "R");
                    print_at (x, y, data);
                    x  = x + 10;
                    itoa (srcreg, data, 10);
                    print_at (x, y, data);
                    break;

                case 07:
                    strcpy (data, "[R");
                    print_at (x, y, data);
                    x = x + 20;
                    itoa (dstreg, data, 10);
                    print_at (x, y, data);
                    if (dstreg >  9)
                    {
                        x = x + 10;
                    }
                    x = x + 10;
                    strcpy (data, "+");
                    print_at (x, y, data);
                    x = x + 10;
                    hexit (x, y, immediate);
                    x = x + 100;
                    strcpy (data, "],");
                    print_at (x, y, data);
                    x = x + 30;
                    strcpy (data, "R");
                    print_at (x, y, data);
                    x  = x + 10;
                    itoa (srcreg, data, 10);
                    print_at (x, y, data);
                    break;
            }
            break;

        case OPCODE_LEA:
            strcpy (data, "R");
            print_at (x, y, data);
            x  = x + 10;
            itoa (dstreg, data, 10);
            print_at (x, y, data);
            if (dstreg >  9)
            {
                x  = x + 10;
            }
            x  = x + 10;
            strcpy (data, ", [R");
            print_at (x, y, data);
            x  = x + 40;
            itoa (srcreg, data, 10);
            print_at (x, y, data);
            if (srcreg >  9)
            {
                x  = x + 10;
            }
            x  = x + 10;

            if (immflag == 1)
            {
                strcpy (data, "+");
                print_at (x, y, data);
                x = x + 10;
                hexit (x, y, immediate);
                x = x + 100;
            }
            strcpy (data, "]");
            print_at (x, y, data);
            break;

        case OPCODE_IN:
            strcpy (data, "R");
            print_at (x, y, data);
            x  = x + 10;
            itoa (dstreg, data, 10);
            print_at (x, y, data);
            if (dstreg  > 9)
            {
                x  = x + 10;
            }
            x  = x + 10;
            strcpy (data, ",");
            print_at (x, y, data);
            x  = x + 20;
            portit (x, y, port);
            break;

        case OPCODE_OUT:
            portit (x, y, port);
            x  = x + 50;
            if (immflag == 1) // we have an immediate value
            {
                strcpy (data, ",");
                print_at (x, y, data);
                x  = x + 20;
                hexit (x, y, immediate);
            }
            else
            {
                strcpy (data, ", R");
                print_at (x, y, data);
                x  = x + 30;
                itoa (srcreg, data, 10);
                print_at (x, y, data);
            }
            break;
    }
}
