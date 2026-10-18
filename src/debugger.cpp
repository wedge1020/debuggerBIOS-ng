////////////////////////////////////////////////////////////////////////////////////////
//
// debugger.cpp - the debugger's interface: the single-step screen, continuation
//                mode, navigation, escape
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
// init(): set up the debugger and the CART's starting context. The CART begins at
// the first word of its ROM with every register 0, except for the stack pointers,
// which point at the base of the CART stack.
//
void Debugger::init (void)
{
    int  *regs    = (int *) ADDR_CART_REGISTERS;
    int   index   = 0;

    pc            = (int *) ADDR_CART_START;
    io_live       = TRUE;       // init_regions() left the hardware as the CART should find it
    clear_seen    = FALSE;
    mode          = MODE_NONE;
    continuing    = FALSE;
    halted        = FALSE;
    rom_end       = ADDR_CART_START + read_port (PORT_CAR_ROMSIZE);

    for (index    = 0;
         index   <  NUM_MODES;
         index    = index + 1)
    {
        view[index]  = 0;
    }
    view[MODE_REG]   = FORMAT_HEX;
    view[MODE_RAM]   = 0;
    view[MODE_STA]   = 9;
    view[MODE_BTR]   = 0;
    view[MODE_INP]   = 0;
    view[MODE_MEM]   = ADDR_MEMCARD;

    history.init (history_store, HISTORY_SIZE);
    trace.init   (trace_store,   TRACE_SIZE);
    input.init   ();

    for (index    = 0;
         index   <  16;
         index    = index + 1)
    {
        regs[index]  = 0;
    }
    regs[14]      = ADDR_CART_STACK_BASE;   // BP
    regs[15]      = ADDR_CART_STACK_BASE;   // SP

    *((int *) ADDR_PORT_TIMFRAME)  = 0;
    *((int *) ADDR_PORT_TIMCYCLE)  = 0;
    *((int *) ADDR_PORT_RNGVALUE)  = read_port (PORT_RNG_VALUE);
    *((int *) ADDR_CART_OFFSET)    = (int) pc;
    *((int *) (ERROR_SCRATCH + 4)) = 0;     // no error being reported
}

////////////////////////////////////////////////////////////////////////////////////////
//
// run(): the debugger never gives control back to the BIOS start-up code; it
// alternates between the single-step screen and continuation mode for as long as
// the console is on (or until ESCAPE).
//
void Debugger::run (void)
{
    while (1)
    {
        if (continuing)
        {
            run_continued ();
        }
        else
        {
            interact ();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// acquire() / release(): who has the GPU (and the selected gamepad). While the
// CART runs, its state is in the hardware. The BIOS takes it over only when it
// has something to draw, saving the CART's state first, and gives it back right
// before the CART runs again. A CART instruction executed after another one then
// costs no port traffic at all.
//
void Debugger::acquire (void)
{
    if (io_live)
    {
        save_cart_io ();
        io_live    = FALSE;
        ui_begin ();
    }
}

void Debugger::release (void)
{
    if (!io_live)
    {
        restore_cart_io ();
        io_live    = TRUE;
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// draw_instruction(): the address and raw words of an instruction, then its
// assembly. Returns the y for the next one.
//
int Debugger::draw_instruction (int *address, int y, int color)
{
    int  instruction   = *address;
    int  immediate     = 0;
    int  immflag       = (instruction >> 25) & 1;

    set_multiply_color (color);
    ui_hex             (0,  y, (int) address, 0.75);
    ui_hex             (88, y, instruction,   0.75);

    if (immflag       == 1)
    {
        immediate      = *(address + 1);
        ui_hex         (0,  y + 18, (int) (address + 1), 0.75);
        ui_hex         (88, y + 18, immediate,           0.75);
    }

    decode             (176, y, instruction, immediate);

    y                  = y + 18;
    if (immflag       == 1)
    {
        y              = y + 18;
    }
    return y;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// draw_history(): the instructions run last, fading out the further back they
// are, and the one about to run in yellow
//
void Debugger::draw_history (void)
{
    int  y        = 50;
    int  shown    = history.count ();
    int  first    = 0;
    int  index    = 0;
    int  level    = 0;

    if (shown    >  HISTORY_SIZE - 1)
    {
        shown     = HISTORY_SIZE - 1;
    }
    first         = history.count () - shown;

    // the instruction words are 32-bit values: ABGR, so the same value in the
    // three low bytes is a gray, and 0xFF in the top one is fully opaque
    for (index    = 0;
         index   <  shown;
         index    = index + 1)
    {
        level     = 0x3F + (32 * index);
        y         = draw_instruction ((int *) history.get (first + index), y,
                                      0xFF000000 | (level << 16) | (level << 8) | level);
    }

    y             = draw_instruction (pc, y, color_yellow);

    if (halted)
    {
        set_multiply_color (color_red);
        ui_print           (0, y + 4, "CART HALTED (HLT)");
    }

    set_multiply_color (color_white);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// adjust(): LEFT/RIGHT (small) and L/R (big) move the current view
//
void Debugger::adjust (int direction, int big)
{
    int  value     = view[mode];
    int  amount    = 16;
    int  last_page = TRACE_SIZE - 16;

    if (big)
    {
        amount     = 256;
    }

    switch (mode)
    {
        case MODE_RAM:
            value  = (value + (direction * amount)) & ADDR_RAM_LAST_ROW;
            break;

        case MODE_MEM:
            value  = ADDR_MEMCARD | ((value + (direction * amount)) & 0x0003FFF0);
            break;

        case MODE_REG:
            if (big)
            {
                value = FORMAT_HEX;
                if (direction > 0)
                {
                    value = NUM_FORMATS - 1;
                }
            }
            else
            {
                value = (value + direction + NUM_FORMATS) % NUM_FORMATS;
            }
            break;

        case MODE_STA:
            amount = 1;
            if (big)
            {
                amount = 3;
            }
            value  = value + (direction * amount);
            if (value <  1)
            {
                value = 1;
            }
            if (value >  18)
            {
                value = 18;
            }
            break;

        case MODE_BTR:
            if (big)
            {
                value = 0;
                if (direction > 0)
                {
                    value = last_page;
                }
            }
            else
            {
                value  = value + (direction * 16);
                if (value <  0)
                {
                    value = last_page;
                }
                if (value >  last_page)
                {
                    value = 0;
                }
            }
            break;

        case MODE_INP:
            if (big)
            {
                value = 0;
                if (direction > 0)
                {
                    value = 3;
                }
            }
            else
            {
                value = (value + direction + 4) % 4;
            }
            break;

        case MODE_RNG:
            // the value the CART will read next from RNG_CurrentValue
            amount = 1;
            if (big)
            {
                amount = 100;
            }
            *((int *) ADDR_PORT_RNGVALUE) =
                *((int *) ADDR_PORT_RNGVALUE) + (direction * amount);
            return;

        default:
            return;
    }

    view[mode]     = value;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// navigate(): act on the navigation controls that fired
//
void Debugger::navigate (int nav)
{
    if ((nav & CTRL_UP) != 0)
    {
        mode       = (mode + 1) % NUM_MODES;
    }

    if ((nav & CTRL_LEFT)  != 0)
    {
        adjust (-1, FALSE);
    }
    if ((nav & CTRL_RIGHT) != 0)
    {
        adjust ( 1, FALSE);
    }
    if ((nav & CTRL_L)     != 0)
    {
        adjust (-1, TRUE);
    }
    if ((nav & CTRL_R)     != 0)
    {
        adjust ( 1, TRUE);
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// interact(): one frame of the single-step screen. The screen is drawn, the
// controls are looked at, and whatever action came out of them is done once the
// frame has been presented.
//
void Debugger::interact (void)
{
    int  action    = ACTION_NONE;

    acquire      ();
    draw_logo    (mode, input.kbd_present);
    draw_history ();
    draw_view    ();

    input.sample ();
    navigate     (input.nav);
    action         = input.action;

    end_frame    ();

    switch (action)
    {
        case ACTION_STEP:
            if (!halted)
            {
                execute (1);
            }
            break;

        case ACTION_CONTINUE:
            if (!halted)
            {
                continuing = TRUE;
            }
            break;

        case ACTION_ESCAPE:
            escape ();
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// run_continued(): CONTINUATION mode. The CART runs, without the debugger screen,
// but is still under control: the controls are looked at every frame, and START
// brings back the single-step screen (X leaves the debugger).
//
// The selected resource view is drawn each time the CART clears the screen, so
// it sits behind whatever the CART draws on top.
//
void Debugger::run_continued (void)
{
    execute (BLOCK_MAX);

    if (clear_seen)
    {
        clear_seen   = FALSE;
        if (mode    != MODE_NONE)
        {
            acquire   ();
            draw_view ();
        }
    }

    if (halted)
    {
        continuing   = FALSE;
        return;
    }

    if (input.sample ())
    {
        if (input.action == ACTION_CONTINUE)
        {
            continuing = FALSE;
        }
        else if (input.action == ACTION_ESCAPE)
        {
            escape ();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// escape(): leave the debugger. The CART's state is put back as it is and
// execution continues at the CART's next instruction, with nothing in between
// any more.
//
// The jump goes through the same RAM routine as everything else, since it has
// to load all 16 registers (jumping needs one, so the routine is the only way).
//
void Debugger::escape (void)
{
    int  *code    = (int *) ADDR_CUSTOM_ROUTINE;

    release ();

    // an impossible CART offset, so an error from here on is reported as it is
    *((int *) ADDR_CART_OFFSET)  = ESCAPED_MARKER;

    code[0]       = WORD_JMP_IMMEDIATE;
    code[1]       = (int) pc;
    code[2]       = WORD_HLT;

    asm
    {
        "MOV R0,  [0x003FFFD0]"
        "MOV R1,  [0x003FFFD1]"
        "MOV R2,  [0x003FFFD2]"
        "MOV R3,  [0x003FFFD3]"
        "MOV R4,  [0x003FFFD4]"
        "MOV R5,  [0x003FFFD5]"
        "MOV R6,  [0x003FFFD6]"
        "MOV R7,  [0x003FFFD7]"
        "MOV R8,  [0x003FFFD8]"
        "MOV R9,  [0x003FFFD9]"
        "MOV R10, [0x003FFFDA]"
        "MOV R11, [0x003FFFDB]"
        "MOV R12, [0x003FFFDC]"
        "MOV R13, [0x003FFFDD]"
        "MOV R14, [0x003FFFDE]"
        "MOV R15, [0x003FFFDF]"
        "JMP 0x003FFF90"
    }
}
