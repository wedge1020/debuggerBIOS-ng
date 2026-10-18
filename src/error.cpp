////////////////////////////////////////////////////////////////////////////////////////
//
// error.cpp - the hardware error screen
//
// NOTE: derived from the error_handler() developed during the spring 2025
// offering of the CSCS2650 Computer Organization course at SUNY CCC; ported over
// to add to the rich functionality of debuggerBIOS.
//
// When the CPU hits an error the console starts the BIOS over at its first word,
// which calls error_handler() with
//
//   R0 = error code     R1 = address of the instruction after the one at fault
//   R2 = the instruction at fault        R3 = its immediate word
//
////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////
//
// section_first() / section_last(): the first and last address of each kind of
// memory, by (address & 0x30000000) >> 28: RAM, BIOS, CART, memory card
//
static int section_first (int section)
{
    return section << 28;
}

static int section_last (int section)
{
    if (section == 0)
    {
        return 0x003FFFFF;
    }
    if (section == 1)
    {
        return 0x100FFFFF;
    }
    if (section == 2)
    {
        return ADDR_CART_START + read_port (PORT_CAR_ROMSIZE) - 1;
    }
    return 0x3003FFFF;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// instruction_before(): the CPU cannot say where the instruction before a given
// one begins, since they are one or two words long. This looks at the flag
// words the way a person would:
//
//   - a word with the immediate flag set just before the address cannot be an
//     instruction (its own immediate would have to be the next word, which is
//     where the instruction at `address` starts), so it is an immediate word and
//     the instruction is the one before it
//   - otherwise it is an instruction, unless the word before it has the flag and
//     is itself an instruction (not an immediate, which the word before that
//     would tell), in which case this one is its immediate
//
// It can be fooled by data, but this is only a best guess for the display.
//
static int instruction_before (int address)
{
    int  *word     = (int *) address;

    if ((word[-1] & INSTR_IMMEDIATE_BIT) != 0)
    {
        return address - 2;
    }

    if ((word[-2] & INSTR_IMMEDIATE_BIT) != 0)
    {
        if ((word[-3] & INSTR_IMMEDIATE_BIT) != 0)
        {
            return address - 1;
        }
        return address - 2;
    }

    return address - 1;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// show_value(): "label = 0x........" in the error colors
//
static void show_value (int y, int *label, int value)
{
    int   buffer[48];
    Text  line;

    line.init  (buffer);
    line.str   (label);
    line.str   (" = ");
    line.hex32 (value);
    ui_print   (49, y, buffer);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// show_instruction(): one line of the listing: address, colon, assembly
//
static int show_instruction (int y, int address, int fault_at)
{
    int   instruction  = *((int *) address);
    int   immediate    = 0;
    int   buffer[12];
    Text  line;

    if (address == fault_at)
    {
        set_multiply_color (color_red);
    }
    else
    {
        set_multiply_color (color_white);
    }

    if ((instruction & INSTR_IMMEDIATE_BIT) != 0)
    {
        immediate      = *((int *) (address + 1));
    }

    line.init  (buffer);
    line.hex32 (address);
    ui_print   (49, y, buffer);
    ui_print (149, y, ":");
    decode  (169, y, instruction, immediate);
    return y + 20;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// report_error(): draw the error screen and halt. It never returns.
//
void report_error (void)
{
    int  *scratch      = (int *) ERROR_SCRATCH;
    int   error_code   = scratch[0];
    int   ip           = scratch[1];
    int   instruction  = scratch[2];
    int   immediate    = scratch[3];
    int  *title        = "UNKNOWN ERROR";
    int  *message      = "Program caused a hardware error with an error\n"
                         "code that was not recognized by the BIOS.";
    int   section      = 0;
    int   first        = 0;
    int   last         = 0;
    int   offset       = 0;
    int   fault_at     = 0;
    int   address      = 0;
    int   y            = 240;
    int   count        = 0;
    int   index        = 0;
    int   buffer[8];
    Text  line;

    // an error while reporting an error: give up and stay on whatever is shown
    if (scratch[4] == ERROR_ACTIVE_MARKER)
    {
        end_frame ();
        asm { "hlt" }
    }
    scratch[4]         = ERROR_ACTIVE_MARKER;

    // ensure everything gets drawn
    end_frame ();

    switch (error_code)
    {
        case ERROR_MEMORY_READ:
            title      = "ERROR: INVALID MEMORY READ";
            message    = "Program attempted to read from a memory address\n"
                         "that does not exist or is in a write-only device.";
            break;

        case ERROR_MEMORY_WRITE:
            title      = "ERROR: INVALID MEMORY WRITE";
            message    = "Program attempted to write on a memory address\n"
                         "that does not exist or is in a read-only device.";
            break;

        case ERROR_PORT_READ:
            title      = "ERROR: INVALID PORT READ";
            message    = "Program attempted to read from a port number\n"
                         "that does not exist or is set as write-only.";
            break;

        case ERROR_PORT_WRITE:
            title      = "ERROR: INVALID PORT WRITE";
            message    = "Program attempted to write on a port number\n"
                         "that does not exist or is set as read-only.";
            break;

        case ERROR_STACK_OVERFLOW:
            title      = "ERROR: STACK OVERFLOW";
            message    = "Program pushed too many values in the stack\n"
                         "and available RAM memory was exhausted.";
            break;

        case ERROR_STACK_UNDERFLOW:
            title      = "ERROR: STACK UNDERFLOW";
            message    = "Program popped too many values from the stack\n"
                         "and all data stored in stack was exhausted.";
            break;

        case ERROR_DIVISION:
            title      = "ERROR: DIVISION BY ZERO";
            message    = "Program attempted to perform a division or\n"
                         "modulus operation where the divisor was zero.";
            break;

        case ERROR_ARC_COSINE:
            title      = "ERROR: ARC COSINE OUT OF RANGE";
            message    = "Program attempted to perform an arc cosine\n"
                         "operation when the argument was not in [-1,+1].";
            break;

        case ERROR_ARC_TANGENT_2:
            title      = "ERROR: ARC TANGENT NOT DEFINED";
            message    = "Program attempted to perform an arc tangent\n"
                         "operation when both of the arguments were 0.";
            break;

        case ERROR_LOGARITHM:
            title      = "ERROR: LOGARITHM OUT OF RANGE";
            message    = "Program attempted to perform a logarithm\n"
                         "operation when the argument is not positive.";
            break;

        case ERROR_POWER:
            title      = "ERROR: POWER HAS NO REAL SOLUTION";
            message    = "Program attempted to perform a power operation\n"
                         "when base was negative and exponent non integer.";
            break;
    }

    draw_message_screen (title, message);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // CART context. An error inside the routine the debugger runs CART code in
    // happens at an address in RAM that means nothing to the CART's author:
    // because the routine is a word for word copy of the CART's code, the
    // address translates exactly. After ESCAPE the CART runs by itself and there
    // is nothing to translate.
    //
    offset             = *((int *) ADDR_CART_OFFSET);
    if ((offset       != ESCAPED_MARKER) &&
        (ip           >  ADDR_CUSTOM_ROUTINE) &&
        (ip           <= (ADDR_CUSTOM_ROUTINE + ROUTINE_WORDS)))
    {
        ip             = offset + (ip - ADDR_CUSTOM_ROUTINE);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // what kind of memory the error is in
    //
    section            = (ip & 0x30000000) >> 28;
    line.init  (buffer);
    line.field ("[RAM] [BIOS][CART][CARD]", section, 6);
    ui_print   (579, 0, buffer);

    set_multiply_color (error_colors_values);
    show_value (160, "Instruction Pointer", ip);
    show_value (180, "Instruction        ", instruction);
    show_value (200, "Immediate Value    ", immediate);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // the instructions around the one at fault (not for the BIOS's own errors):
    // up to two before it, the one itself in red, up to two after
    //
    if (section       != 1)
    {
        first          = section_first (section);
        last           = section_last  (section);
        fault_at       = ip - 1 - ((instruction >> 25) & 1);

        // before: found backwards, so remembered first and then listed
        address        = fault_at;
        count          = 0;
        while ((count  < 2) &&
               ((address - 3) >= first) &&
               (address       <= last))
        {
            address    = instruction_before (address);
            count      = count + 1;
        }

        if ((fault_at >= first) &&
            (fault_at <= last))
        {
            // list from the earliest one found up to two after the instruction
            // at fault
            index      = 0;
            while ((index < (count + 3)) &&
                   (address >= first) &&
                   (address <= last))
            {
                if ((address + ((*((int *) address) >> 25) & 1)) > last)
                {
                    break;
                }
                y          = show_instruction (y, address, fault_at);
                address    = address + 1 + ((*((int *) address) >> 25) & 1);
                index      = index + 1;
            }
        }
    }

    set_multiply_color (color_white);
    ui_print           (579, 339, "[HALT]");

    // stop any sound
    stop_all_channels ();

    end_frame ();
    asm { "hlt" }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// error_handler(): the entry point the hardware uses. It does nothing but stash
// R0-R3 (the only values that matter) and move to the debugger's own stack
// before calling the function that reports, so the report does not depend on
// the CART's stack being in a condition to use.
//
// The name is fixed by the BIOS: the compiler looks for a function called
// exactly error_handler.
//
void error_handler ()
{
    asm
    {
        "MOV  [0x003FFFF0], R0"
        "MOV  [0x003FFFF1], R1"
        "MOV  [0x003FFFF2], R2"
        "MOV  [0x003FFFF3], R3"
        "MOV  R14,          0x003FFF8F"
        "MOV  R15,          0x003FFF8F"
    }

    report_error ();
}
