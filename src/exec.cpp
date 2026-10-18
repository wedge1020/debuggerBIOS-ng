////////////////////////////////////////////////////////////////////////////////////////
//
// exec.cpp - running the CART under the debugger
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
// readable(): can the CPU read this address without an error? (the checks a
// CART instruction fetch is subject to). Used before the debugger touches an
// address the CART gave it, so a bad jump is reported as the CART's error and
// not as a failure of the BIOS.
//
int Debugger::readable (int address)
{
    if ((address >= 0) &&
        (address <= 0x003FFFFF))
    {
        return TRUE;
    }

    if ((address >= 0x10000000) &&
        (address <= 0x100FFFFF))
    {
        return TRUE;
    }

    if ((address >= ADDR_CART_START) &&
        (address <  rom_end))
    {
        return TRUE;
    }

    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// fault(): the CART did something the console would have reported as an error.
// Do the same: report it exactly as the error handler would for the hardware,
// with the values it would be given (error code, the address after the
// instruction, the instruction, its immediate word), and halt.
//
void Debugger::fault (int code, int instruction, int immediate)
{
    int  *scratch  = (int *) ERROR_SCRATCH;

    scratch[0]     = code;
    scratch[1]     = (int) (pc + 1 + ((instruction >> 25) & 1));
    scratch[2]     = instruction;
    scratch[3]     = immediate;

    // the address is the CART's own: there is nothing to translate
    *((int *) ADDR_CART_OFFSET)  = ESCAPED_MARKER;

    report_error ();
}

////////////////////////////////////////////////////////////////////////////////////////
//
// run_routine(): run the routine in RAM with the CART's registers, and come back.
//
// The routine ends by jumping to _CUSTOM_RET (its address is stored in RAM for
// the routine to pick up), where the CART's registers are saved. Only the BIOS
// BP and SP are kept across the trip: the compiler never leaves anything in the
// other registers between statements, and this is all one statement.
//
// The routine is not CALLed, even though that would be tidier: a CALL would push
// a return address on the CART stack. The "PUSH R10" at the end of the routine
// is the only trace left there, and it is popped right away.
//
void Debugger::run_routine (void)
{
    asm
    {
        "MOV  [0x003FFFFE], R14"        // BIOS BP
        "MOV  [0x003FFFFF], R15"        // BIOS SP
        "MOV  R0,           _CUSTOM_RET"
        "MOV  [0x003FFFEF], R0"         // where the routine returns to

        "MOV  R13,          [0x003FFFDD]"
        "MOV  R12,          [0x003FFFDC]"
        "MOV  R11,          [0x003FFFDB]"
        "MOV  R10,          [0x003FFFDA]"
        "MOV  R9,           [0x003FFFD9]"
        "MOV  R8,           [0x003FFFD8]"
        "MOV  R7,           [0x003FFFD7]"
        "MOV  R6,           [0x003FFFD6]"
        "MOV  R5,           [0x003FFFD5]"
        "MOV  R4,           [0x003FFFD4]"
        "MOV  R3,           [0x003FFFD3]"
        "MOV  R2,           [0x003FFFD2]"
        "MOV  R1,           [0x003FFFD1]"
        "MOV  R0,           [0x003FFFD0]"
        "MOV  R14,          [0x003FFFDE]"
        "MOV  R15,          [0x003FFFDF]"
        "JMP  0x003FFF90"

        "_CUSTOM_RET:"
        "POP  R10"                      // undo the routine's PUSH R10
        "MOV  [0x003FFFDD], R13"
        "MOV  [0x003FFFDC], R12"
        "MOV  [0x003FFFDB], R11"
        "MOV  [0x003FFFDA], R10"
        "MOV  [0x003FFFD9], R9"
        "MOV  [0x003FFFD8], R8"
        "MOV  [0x003FFFD7], R7"
        "MOV  [0x003FFFD6], R6"
        "MOV  [0x003FFFD5], R5"
        "MOV  [0x003FFFD4], R4"
        "MOV  [0x003FFFD3], R3"
        "MOV  [0x003FFFD2], R2"
        "MOV  [0x003FFFD1], R1"
        "MOV  [0x003FFFD0], R0"
        "MOV  [0x003FFFDE], R14"
        "MOV  [0x003FFFDF], R15"
        "MOV  R14,          [0x003FFFFE]"
        "MOV  R15,          [0x003FFFFF]"
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// emulate(): carry out an instruction the debugger must not let the CPU run.
//
//   JMP CALL RET JT JF  would move the real instruction pointer out of the BIOS
//   IN                  of the timer and RNG ports: the CART must see its own
//                       frame and cycle counts (the debugger spends frames the
//                       CART never knew about) and the random number the
//                       debugger lets the user choose
//   HLT                 would stop the console, and the debugger with it
//
// CALL pushes the address of the instruction after it, as the CPU does, and RET
// pops it into the program counter, so the stack the CART (and the stack view)
// sees is the real one.
//
void Debugger::emulate (int instruction, int opcode)
{
    int  *regs     = (int *) ADDR_CART_REGISTERS;
    int   immflag  = (instruction >> 25) & 1;
    int   dstreg   = (instruction >> 21) & 15;
    int   srcreg   = (instruction >> 17) & 15;
    int   immediate = 0;
    int  *next     = pc + 1 + immflag;
    int  *target   = next;
    int   stack    = 0;
    int   port     = 0;

    if (immflag   == 1)
    {
        immediate  = *(pc + 1);
    }

    // note it in the history (the instructions run in a block are noted by
    // execute(), which runs them)
    if (opcode != OPCODE_HLT)
    {
        history.push ((int) pc);
    }

    switch (opcode)
    {
        case OPCODE_HLT:
            halted      = TRUE;
            continuing  = FALSE;
            return;

        case OPCODE_JMP:
            target      = (int *) immediate;
            if (immflag == 0)
            {
                target  = (int *) regs[dstreg];
            }
            break;

        case OPCODE_CALL:
            target      = (int *) immediate;
            if (immflag == 0)
            {
                target  = (int *) regs[dstreg];
            }
            stack       = regs[15] - 1;
            if (stack   <  0)
            {
                fault (ERROR_STACK_OVERFLOW, instruction, immediate);
            }
            if (stack   >  0x003FFFFF)
            {
                fault (ERROR_MEMORY_WRITE, instruction, immediate);
            }
            *((int *) stack)  = (int) next;
            regs[15]          = stack;
            trace.push ((int) target);
            break;

        case OPCODE_RET:
            stack       = regs[15];
            if (stack   >  0x003FFFFF)
            {
                fault (ERROR_STACK_UNDERFLOW, instruction, immediate);
            }
            if (stack   <  0)
            {
                fault (ERROR_MEMORY_READ, instruction, immediate);
            }
            target      = (int *) (*((int *) stack));
            regs[15]    = stack + 1;
            trace.pop ();
            break;

        case OPCODE_JT:
            if (regs[dstreg] != 0)
            {
                target  = (int *) immediate;
                if (immflag == 0)
                {
                    target = (int *) regs[srcreg];
                }
            }
            break;

        case OPCODE_JF:
            if (regs[dstreg] == 0)
            {
                target  = (int *) immediate;
                if (immflag == 0)
                {
                    target = (int *) regs[srcreg];
                }
            }
            break;

        case OPCODE_IN:
            port        = instruction & INSTR_PORT_MASK;
            if (port   == PORT_TIM_FRAMECOUNTER)
            {
                regs[dstreg]  = *((int *) ADDR_PORT_TIMFRAME);
            }
            else if (port == PORT_TIM_CYCLECOUNTER)
            {
                regs[dstreg]  = *((int *) ADDR_PORT_TIMCYCLE);
            }
            else if (port == PORT_INP_SELECTED)
            {
                regs[dstreg]  = *((int *) (ADDR_SHADOW + SHADOW_GAMEPAD));
            }
            else
            {
                // RNG_CurrentValue: hand over the value held in memory, then
                // draw the next one
                regs[dstreg]  = *((int *) ADDR_PORT_RNGVALUE);
                *((int *) ADDR_PORT_RNGVALUE) = read_port (PORT_RNG_VALUE);
            }
            break;

        case OPCODE_OUT:
            // OUT INP_SelectedGamepad: the console ignores values out of range
            port        = instruction & INSTR_PORT_MASK;
            immediate   = immflag ? immediate : regs[srcreg];
            if ((immediate >= 0) &&
                (immediate <  4))
            {
                *((int *) (ADDR_SHADOW + SHADOW_GAMEPAD)) = immediate;
                if (io_live)
                {
                    select_gamepad (immediate);
                }
            }
            break;
    }

    pc                          = target;
    *((int *) ADDR_PORT_TIMCYCLE) = *((int *) ADDR_PORT_TIMCYCLE) + 1;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// execute(): run up to `limit` CART instructions, starting at `pc`.
//
// Plain instructions are copied into the routine at ADDR_CUSTOM_ROUTINE and
// run together. Because the copy keeps every word at the same distance from the
// start as it had in the CART, a CPU error inside the routine can be mapped back
// to the exact CART address (see error_handler()).
//
// A run stops, with the instruction that caused it left for the next call:
//
//   - at an instruction to emulate (see emulate()); when it is the first one,
//     it is emulated here and now
//   - after a WAIT (the end of a CART frame: the frame and cycle counters the
//     CART sees are updated)
//   - after a command that clears the screen (the debugger draws its content
//     view right then, in continuation mode)
//   - after an OUT to the GPU command port that takes its value from a register,
//     when it is not first: the register may be changed by what is ahead of it
//     in the run, so the value is only known once those have run
//
void Debugger::execute (int limit)
{
    int  *code      = (int *) ADDR_CUSTOM_ROUTINE;
    int  *regs      = (int *) ADDR_CART_REGISTERS;
    int  *source    = pc;
    int   words     = 0;
    int   count     = 0;
    int   instruction = 0;
    int   opcode    = 0;
    int   immflag   = 0;
    int   port      = 0;
    int   value     = 0;
    int   ending    = 0;    // 1: WAIT is the last of the run, 2: a screen clear is
    int   emulating = FALSE;
    int   stop      = FALSE;

    // a jump to an address the console would not let the CART read from
    if (!readable ((int) source))
    {
        fault (ERROR_MEMORY_READ, 0, 0);
    }

    while ((count < limit) &&
           (stop == FALSE))
    {
        instruction  = *source;
        opcode       = (instruction >> 26) & 0x3F;
        immflag      = (instruction >> 25) & 1;
        emulating    = FALSE;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // the few instructions that are not just copied (all of them have
        // opcodes of 0x18 or below)
        //
        if ((opcode <= OPCODE_OUT) &&
            (((SPECIAL_OPCODES >> opcode) & 1) != 0))
        {
            port        = instruction & INSTR_PORT_MASK;

            if (opcode == OPCODE_WAIT)
            {
                ending  = 1;
            }

            else if (opcode == OPCODE_IN)
            {
                emulating = (port == PORT_TIM_FRAMECOUNTER) ||
                            (port == PORT_TIM_CYCLECOUNTER) ||
                            (port == PORT_RNG_VALUE)        ||
                            (port == PORT_INP_SELECTED);
            }

            else if (opcode == OPCODE_OUT)
            {
                if (port == PORT_INP_SELECTED)
                {
                    // the selected gamepad cannot be read back reliably, so
                    // the debugger keeps track of what the CART selects
                    emulating = TRUE;
                }
                else if (port == PORT_GPU_COMMAND)
                {
                    if (immflag == 1)
                    {
                        value   = *(source + 1);
                        if (value == GPUCMD_CLEAR)
                        {
                            ending  = 2;
                        }
                    }
                    else if (count > 0)
                    {
                        stop    = TRUE;
                    }
                    else
                    {
                        value   = regs[(instruction >> 17) & 15];
                        if (value == GPUCMD_CLEAR)
                        {
                            ending  = 2;
                        }
                    }
                }
            }

            else
            {
                emulating = TRUE;
            }
        }

        if (emulating)
        {
            if (count == 0)
            {
                emulate (instruction, opcode);
                return;
            }
            stop = TRUE;
        }

        if (stop == FALSE)
        {
            ////////////////////////////////////////////////////////////////////////////
            //
            // copy the instruction (and its immediate) into the routine, and
            // note it in the history
            //
            history.store[history.head]  = (int) source;
            history.head                 = (history.head + 1) & history.mask;

            code[words]  = instruction;
            words        = words + 1;
            if (immflag == 1)
            {
                code[words]  = *(source + 1);
                words        = words + 1;
                source       = source + 1;
            }
            source       = source + 1;
            count        = count + 1;

            if (ending  != 0)
            {
                stop     = TRUE;
            }
        }
    }

    if (count == 0)
    {
        return;
    }

    history.used  = history.used + count;
    if (history.used > history.capacity ())
    {
        history.used = history.capacity ();
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // after the CART's words: restore R10 (the routine uses it to jump back)
    //
    //   PUSH R10 / MOV R10, [return address] / JMP R10 / HLT
    //
    code[words]      = WORD_PUSH_R10;
    code[words + 1]  = WORD_MOV_R10_MEM;
    code[words + 2]  = ADDR_CUSTOM_RETURN;
    code[words + 3]  = WORD_JMP_R10;
    code[words + 4]  = WORD_HLT;

    // for error_handler(): where this run began in the CART
    *((int *) ADDR_CART_OFFSET)  = (int) pc;

    release ();
    run_routine ();

    pc               = source;
    *((int *) ADDR_PORT_TIMCYCLE) = *((int *) ADDR_PORT_TIMCYCLE) + count;

    if (ending       == 1)
    {
        *((int *) ADDR_PORT_TIMFRAME) = *((int *) ADDR_PORT_TIMFRAME) + 1;
        *((int *) ADDR_PORT_TIMCYCLE) = 0;
    }
    else if (ending  == 2)
    {
        clear_seen   = TRUE;
    }
}
