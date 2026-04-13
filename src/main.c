////////////////////////////////////////////////////////////////////////////////////////
//
// main(): normal BIOS routine run on system startup
//
void main (void)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Declare variables - note their position relative to BP
    //
    int     *code;        // [BP-1]: pointer to our dynamic code
    int     *offset;      // [BP-2]: address of instruction to process
    int      instruction; // [BP-3]: raw data of instruction
    int      immediate;   // [BP-4]: immediate value of instruction
    int      immflag;     // [BP-5]: immediate flag of instruction
    int      index;       // [BP-6]: index for loops and sequences
    int      y;
    int [8]  history;     // previous instructions to display
    int      count;       // tracking history array content quantity
    int      pos;         // another index variable
    int     *address;
    int     *coffset;     // pointer to C code offset array
    int     *ccode;       // pointer to embedded C code
    int     *ctmp;        // pointer to embedded C code
    int     *cotmp;       // pointer to embedded C code offset
    int      slen;
    int      num_offsets;
    int      value;
    int      framestop;
    int      stepflag;
    int      modeflag;
    int      codemode;
    int      yflag;
    int      continueflag;
    int      exitflag;
    int      upflag;
    int      incflag;
    int      decflag;
    int      clearflag;
    int      waitflag;
    int      memstart;
    int      stackgap;
    int      gamepad;
    int      cardstart;
    int      color;
    int      srcreg;
    int      dstreg;
    int      port;
    int    **mem;
    int [64] backtrace;
    int      btrace;
    int      btstart;
    int [16] chistory;     // previous instructions to display
    int      ccount;       // tracking history array content quantity
    int [16] clhistory;
    int      cstepflag;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // init_regions(): Move  all general BIOS actions  (defining regions,
    // checking for cartridge, etc.) to its own subroutine, to facilitate
    // debugger debugging
    //
    init_regions ();

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // debuggerBIOS memory map:
    //
    // 0x003FFB9F base of cartridge stack (initial CART BP/SP)
    // 0x003FFBA0    (R0)  value from the context of the CART ROM
    // 0x003FFBA1    (R1)  value from the context of the CART ROM
    // 0x003FFBA2    (R2)  value from the context of the CART ROM
    // 0x003FFBA3    (R3)  value from the context of the CART ROM
    // 0x003FFBA4    (R4)  value from the context of the CART ROM
    // 0x003FFBA5    (R5)  value from the context of the CART ROM
    // 0x003FFBA6    (R6)  value from the context of the CART ROM
    // 0x003FFBA7    (R7)  value from the context of the CART ROM
    // 0x003FFBA8    (R8)  value from the context of the CART ROM
    // 0x003FFBA9    (R9)  value from the context of the CART ROM
    // 0x003FFBAA    (R10) value from the context of the CART ROM
    // 0x003FFBAB CR (R11) value from the context of the CART ROM
    // 0x003FFBAC SR (R12) value from the context of the CART ROM
    // 0x003FFBAD DR (R13) value from the context of the CART ROM
    // 0x003FFBAE BP (R14) value from the context of the CART ROM
    // 0x003FFBAF SP (R15) value from the context of the CART ROM
    // 0x003FFBB0 top of debugger stack (1023 total words)
    //   . . .
    // 0x003FFFAF base of debugger/BIOS stack
    // 0x003FFFB0 start of dynamic subroutine (50 total words)
    //   . . .
    // 0x003FFFE2 end of dynamic subroutine
    // 0x003FFFE3 CART GPU_ClearColor value
    // 0x003FFFE4 CART GPU_MultiplyColor value
    // 0x003FFFE5 CART GPU_ActiveBlending value
    // 0x003FFFE6 CART texture id
    // 0x003FFFE7 CART region id
    // 0x003FFFE8 CART GPU_DrawingPointX
    // 0x003FFFE9 CART GPU_DrawingPointY
    // 0x003FFFEA CART scaleX
    // 0x003FFFEB CART scaleY
    // 0x003FFFEC CART angle
    // 0x003FFFED CART gamepad id
    // 0x003FFFEE offset of current CART instruction
    // 0x003FFFEF address of where our jumped-to routine will "return" to
    // 0x003FFFF0    (R0)  value from the context of the BIOS ROM
    // 0x003FFFF1    (R1)  value from the context of the BIOS ROM
    // 0x003FFFF2    (R2)  value from the context of the BIOS ROM
    // 0x003FFFF3    (R3)  value from the context of the BIOS ROM
    // 0x003FFFF4    (R4)  value from the context of the BIOS ROM
    // 0x003FFFF5    (R5)  value from the context of the BIOS ROM
    // 0x003FFFF6    (R6)  value from the context of the BIOS ROM
    // 0x003FFFF7    (R7)  value from the context of the BIOS ROM
    // 0x003FFFF8    (R8)  value from the context of the BIOS ROM
    // 0x003FFFF9    (R9)  value from the context of the BIOS ROM
    // 0x003FFFFA    (R10) value from the context of the BIOS ROM
    // 0x003FFFFB CR (R11) value from the context of the BIOS ROM
    // 0x003FFFFC SR (R12) value from the context of the BIOS ROM
    // 0x003FFFFD DR (R13) value from the context of the BIOS ROM
    // 0x003FFFFE BP (R14) value from the context of the BIOS ROM
    // 0x003FFFFF SP (R15) value from the context of the BIOS ROM
    //
    // This will allow the debugger  to maintain important data, separate
    // whatever the  CART ROM ends up  doing (provided it plays  nice and
    // just uses BP/SP as presented).
    //
    // We should be able  to scale this as needed, but  the aim should be
    // to allocate  as little memory  as possible, to reduce  issues with
    // the CART ROM, which will have no idea what is going on.
    //
    // As heavier functions like decode() add close to 500 words onto the
    // stack, so some further memory map adjustments had to be done. BIOS
    // stack was  upgraded from 160 words  to just shy of  1024 words (at
    // 1023 words). This hopefully will be more than enough.
    //
    ////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up desired stack configuration  - we want nothing initially on
    // the stack, so we'll discard that return address.
    //
    // Since the point  is to never return from main(),  before any local
    // variables are declared,  let us do some  rearranging to facilitate
    // our operations here, in accordance with the memory map above:
    //
    // Process: determine the  difference between the old BP  and our new
    // one; then just subtract that difference from the SP (which will be
    // at the  correct value  based on variables  declared), so  it'll be
    // adjusted into the  new stack space at the  appropriate offset from
    // BP,  eliminating any  future issues  when declaring  (or removing)
    // variables at the start of main().
    //
    asm
    {
        "PUSH R10"                       // back up R10 (arbitrary register)
        "MOV  R10,          R14"         // copy base pointer into R10
        "MOV  R14,          0x003FFFAF"  // new stack base
        "MOV  [0x003FFFFE], R14"         // back up debug/BIOS BP
        "ISUB R10,          R14"         // subtract new BP from old BP
        "ISUB R15,          R10"         // adjust SP to new offset
        "MOV  [0x003FFFFF], R15"         // back up debug/BIOS SP
        "MOV  R10,          0x003FFB9F"  // CART initial BP
        "MOV  [0x003FFBAE], R10"         // store initial CART BP
        "MOV  R10,          0x003FFB9F"  // CART initial SP (offset for variables)
        "MOV  [0x003FFBAF], R10"         // store initial CART SP
        "POP  R10"                       // restore R10
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // DEBUG MODE: C CODE vs ASM
    //
    // After some extensive collating of the .debug files, and some binary
    // hacking, any available C code (as mapped to an offset) is now included
    // in the CART. This section will set up the check to see if C debug data
    // is available, and switch to C debugging mode if so (Y button will then
    // be used to toggle between C DEBUG mode and ASM DEBUG mode).
    //
    asm
    {
        "PUSH  R9"
        "PUSH  R10"
        "IN    R9,        CAR_ProgramROMSize"
        "IADD  R9,        0x20000000"
        "ISUB  R9,        1"
        "MOV   R10,       [R9]"
        "ISUB  R9,        R10"
        "IADD  R9,        1"
        //"MOV   R10,       [R9]"
        "MOV   {coffset}, R9"
        "POP   R10"
        "POP   R9"
    }

    codemode                           = DEBUG_ASM;
    if (((int)coffset                 >= 0x20000000) &&
        ((int)coffset                 <= 0x27FFFFFF))
    {
        if (*coffset                  == 0x54584554) // if "TEXT" is here
        {
            codemode                   = DEBUG_C;
            coffset                    = coffset + 1;
            ccode                      = coffset + (int) *coffset;
            ccode                      = ccode + 2;
            cstepflag                  = TRUE;
        }
    }
    else
    {
        coffset                        = NULL;
        ccode                          = NULL;
        cstepflag                      = FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // DEBUGGER: instead of directly passing control off to the CART ROM,
    // we will retain  control here in the BIOS,  providing the debugging
    // interface, allowing for finer instruction-by-instruction analysis.
    //
    ////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // We will need a variable, an  integer pointer (offset) to track our
    // current position in  CART ROM execution (our  program data). Think
    // of it like our own program counter.
    //
    // ... in an earlier commit, this was done in assembly like so:
    //
    // asm
    // {
    //     "mov   R0,       0x20000000"
    //     "mov   {offset}, R0"
    // }
    //
    offset                             = (int *) 0x20000000; // first CART word

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // debugging  loop.  This  is  an   infinite  while  loop  that  will
    // continually take us from instruction to instruction.
    //
    count                              = 0;
    ccount                             = 0;
    modeflag                           = MODE_NONE; // no content view by default
    memstart                           = 0x003FFFF0;
    cardstart                          = 0x30000000;
    stackgap                           = 9;
    exitflag                           = FALSE;
    continueflag                       = CONTINUE_NONE;
    clearflag                          = FALSE;
    waitflag                           = FALSE;
    for (index                         = 0;
         index                        <  64;
         index                         = index + 1)
    {
        backtrace[index]               = -1;
    }
    btrace                             = 0;
    btstart                            = 0;
    while (1)
    {
        ////////////////////////////////////////////////////////////////////////////////
        //
        // For error_handler() enhancement:  back up the next CART offset
        // to be processed.  In the event of an  error, this value can be
        // be read and used to display CART-context instructions
        //
        // 0x003FFFEE offset of current CART instruction
        //
        // This  was  originally done  in  ASM,  but  then redone  in  C.
        // Provided here  is that original  inline ASM (after  setting up
        // code to store the address of the next instruction to execute):
        //
        // asm
        // {
        //     "push  R0"
        //     "mov   R0,           {code}" // doing an implied dereference?
        //     "mov   [0x003FFFEE], R0"
        //     "pop   R0"
        // }
        //
        instruction                    = *offset;
        immflag                        = instruction & 0x02000000;
        code                           = (int *) ADDR_CART_OFFSET;
        *code                          = (int) offset + 1;
        if (immflag                   >  0)
        {
            *code                      = (int) offset + 2;
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // continue  mode view  refresh: only  render once  the CART  has
        // issued  a  clearing  of  the  screen  (via  `out  GPU_Command,
        // GPUCommand_ClearScreen`)
        //
        if ((continueflag             == CONTINUE_ENABLED) &&
            (clearflag                == TRUE))
        {
            views (modeflag, offset, memstart, stackgap, gamepad, cardstart, backtrace, btstart);
            clearflag                  = FALSE;
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // START: remain  in debugger,  but continue normal  execution of
        // CART instructions (occurs on release after press)
        //
        value                          = gamepad_button_start ();
        if ((value                    >= BUTTON_IS_PRESSED) &&
            (continueflag             == CONTINUE_ENABLED))
        {
            continueflag               = CONTINUE_DETRIGGER;
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // START: recognize key release  after being pressed (the setting
        // of continueflag to 2 allows framestop to continually be set to
        // 0, prevently a re-entry of  the single-step prompt). Note that
        // with  this, we  are still  in the  debugging monitor,  and can
        // break back to the single-step prompt by pressing START again.
        //
        else if ((value               <= BUTTON_NOT_PRESSED) &&
                 (continueflag        == CONTINUE_DETRIGGER))
        {
            continueflag               = CONTINUE_NONE;
            framestop                  = -1;
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // debugger interactive interface (for single-step mode)
        //
        framestop                      = -1;
        stepflag                       = FALSE;
        upflag                         = FALSE;
        yflag                          = FALSE;
        incflag                        = FALSE;
        decflag                        = FALSE;
        exitflag                       = FALSE;
        if (continueflag              == CONTINUE_ENABLED)
        {
            framestop                  = 0;
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // update the history array, cycling out the old, bringing in the
        // new as needed
        //
        if (count                     == 8) // buffer the previous 7 instructions,
        {                                   // cycling off the ones as needed
            for (index                 = 1;
                 index                <  8;
                 index                 = index + 1)
            {
                history[index-1]       = history[index]; // oldest one goes away
            }
            count                      = count - 1; // allow new instruction
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // C: update the C history array, cycling out the old, bringing in the
        // new as needed
        //
        if (ccount                    == 16) // buffer the previous 15 instructions,
        {                                   // cycling off the ones as needed
            for (index                 = 1;
                 index                <  16;
                 index                 = index + 1)
            {
                chistory[index-1]      = chistory[index];  // oldest one goes away
                clhistory[index-1]     = clhistory[index]; // oldest one goes away
            }
            ccount                     = ccount - 1;       // allow new instruction
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // C: if C debugging is available, tend to the chistory array, filling it
        // ONLY with those offsets that actually match an available C line string
        //
        if (codemode                  == DEBUG_C)
        {
            address                    = coffset + 1;
            num_offsets                = *coffset;  // how many offsets
            ctmp                       = ccode;     // point at first string offset
            slen                       = *(ctmp-1); // get the string length
            for (pos                   = 0;
                 pos                  <  num_offsets;
                 pos                   = pos + 1)
            {
                ////////////////////////////////////////////////////////////////////////
                //
                // address is the address within the array of offsets with
                // links to C code. The variable value is the actual offset
                // we can use for comparison purposes
                //
                value                  = *address;

                ////////////////////////////////////////////////////////////////////////
                //
                // does the current address value match the current instruction
                // offset being processed? If so, we have a match
                //
                if (value             == (int) offset)
                {
                    asm { "_ABC:" }
                    chistory[ccount]   = (int) offset;
                    clhistory[ccount]  = (int) ctmp;
                    ccount             = ccount + 1;
                    cstepflag          = FALSE;
                    break;
                }

                ////////////////////////////////////////////////////////////////////////
                //
                // As the offsets are in incrementing order, once value exceeds
                // the offset in question, we do not need to go any further:
                // bail out
                //
                else if (value        >  (int) offset)
                {
                    break;
                }

                address                = address + 1;
                ctmp                   = ctmp + slen; // hop to the next string offset
                slen                   = *ctmp;       // get the new string word length
                ctmp                   = ctmp + 1;    // position at start of next string
            }
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // ASM: obtain the next instruction from our CART offset (always happens
        // regardless of debugging mode, since this is what occurs on the CPU)
        //
        history[count]                 = (int) offset;
        count                          = count + 1;

        if (cstepflag                 == TRUE)
        {
            framestop                  = 1;
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // single-step loop
        //
        while (framestop              != 0) // single step loop
        {
            ////////////////////////////////////////////////////////////////////////////
            //
            // draw "debuggerBIOS" logo in top left, with instructions
            //
            draw_logo (modeflag, coffset);

            ////////////////////////////////////////////////////////////////////////////
            //
            // display   the   instruction,  highlighting   the   current
            // one  yellow;   past  instructions   are  displayed   in  a
            // fading-from-white gradient
            //
            y                          = 50;
            if (codemode              == DEBUG_ASM)
            {
                for (index             = 0;
                     index            <  count;
                     index             = index + 1)
                {
                    address            = (int *) history[index];
                    instruction        = *address;
                    immflag            = instruction & 0x02000000;
                    if (immflag       >  0)                         // immediate bit
                    {
                        immediate      = *(address+1);              // deref offset
                    }
                    else
                    {
                        immediate      = 0;                             // no immediate
                    }

                    if (index         == (count - 1))
                    {
                        set_multiply_color (color_yellow);
                    }
                    else
                    {
                        ////////////////////////////////////////////////////////////////
                        //
                        // calculate white  fade to black: as  we are dealing
                        // with raw binary  data store on the  machine, it is
                        // encoded in the machine's  little endian format, so
                        // the word  representing the RGBA value  is actually
                        // of the form ABGR (note the shifts below)
                        //
                        value          = 0x3F + (8 * index);
                        color          = (((value + (index * 24))));       // RED
                        color         |= (((value + (index * 24))) << 8);  // GREEN
                        color         |= (((value + (index * 24))) << 16); // BLUE
                        color         |= (((0xFF))                 << 24); // ALPHA
                        set_multiply_color (color);
                    }

                    hexit_zoomed (0,  y, (int) address, 0.75); // instruction addr
                    hexit_zoomed (88, y, instruction,   0.75); // instruction hex

                    ////////////////////////////////////////////////////////////////////
                    //
                    // as some instructions may  have immediate data, we need
                    // to check  if the  current instruction  has it,  and to
                    // obtain  it if  so (if  not, set  'immediate' to  0 and
                    // otherwise ignore it).
                    //
                    // We do this by masking  out all bits in the instruction
                    // save for the immediate flag. If the resulting value is
                    // greater than 0,  that bit is set, that  means there is
                    // immediate data.
                    //
                    // This is what  we came up with initially  in class, but
                    // then I later  realized we can do this in  C, as we did
                    // above for instruction:
                    //
                    // asm
                    // {
                    //    "mov   R0,          {offset}"
                    //    "mov   R1,          [R0]"
                    //    "mov   {immediate}, R1"
                    // }
                    //
                    if (immflag       >  0)                         // immediate bit
                    {
                        address        = address + 1;
                        hexit_zoomed (0, (y + 18), (int)address, 0.75); // immediate

                        hexit_zoomed (88, (y + 18), immediate, 0.75);
                    }

                    decode (176, y, instruction, immediate);
                    if (index         == (count - 1))
                    {
                        set_multiply_color (color_white);
                    }

                    y                  = y + 18;
                    if (immflag       >  0)
                    {
                        y              = y + 18;
                    }
                }
            }

            else if (codemode         == DEBUG_C)
            {
                if (ccount            == 0)
                {
                    cstepflag          = TRUE; // short-circuit pressing of down
                }

                for (index             = 0;
                     index            <  ccount;
                     index             = index + 1)
                {
                    if (index         == (ccount - 1))
                    {
                        set_multiply_color (color_yellow);
                    }
                    else
                    {
                        ////////////////////////////////////////////////////////////////
                        //
                        // calculate white  fade to black: as  we are dealing
                        // with raw binary  data store on the  machine, it is
                        // encoded in the machine's  little endian format, so
                        // the word  representing the RGBA value  is actually
                        // of the form ABGR (note the shifts below)
                        //
                        value          = 0x3F + (16 * index);
                        color          = (((value + (index * 24))));       // RED
                        color         |= (((value + (index * 24))) << 8);  // GREEN
                        color         |= (((value + (index * 24))) << 16); // BLUE
                        color         |= (((0xFF))                 << 24); // ALPHA
                        set_multiply_color (color);
                    }

                    asm { "_DEF:" }
                    address            = (int *) clhistory[index];
                    zprint_zoomed_at (0, y, address, 0.75);

                    if (index         == (ccount - 1))
                    {
                        set_multiply_color (color_white);
                    }

                    y                  = y + 18;
                }
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // resource view logic
            //
            views (modeflag, offset, memstart, stackgap, gamepad, cardstart, backtrace, btstart);

            ////////////////////////////////////////////////////////////////////////////
            //
            // Y: toggle debugging modes (C vs ASM)
            //
            value                      = gamepad_button_y ();
            if ((value                >= BUTTON_IS_PRESSED) &&
                (yflag                == FALSE))
            {
                codemode               = (codemode + 1) % 2;
                yflag                  = TRUE;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // UP: recognize key release after being pressed
            //
            else if ((value           <= BUTTON_NOT_PRESSED) &&
                (yflag                == TRUE))
            {
                yflag                  = FALSE;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // UP: cycle content display
            //
            value                      = gamepad_up ();
            if ((value                >= BUTTON_IS_PRESSED) &&
                (upflag               == FALSE))
            {
                modeflag               = (modeflag + 1) % MAX_MODES;
                upflag                 = TRUE;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // UP: recognize key release after being pressed
            //
            else if ((value           <= BUTTON_NOT_PRESSED) &&
                (upflag               == TRUE))
            {
                upflag                 = FALSE;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // LEFT: decrement content by 16
            //
            value                      = gamepad_left ();
            if ((value                >= BUTTON_IS_PRESSED) &&
                (decflag              == FALSE))
            {
                decflag                = TRUE;
                if (modeflag          == MODE_MEMORY)
                {
                    memstart           = memstart - 16;
                    if (memstart      <  0x00000000)
                    {
                        memstart       = 0x003FFFF0;
                    }
                }
                else if (modeflag     == MODE_STACK)
                {
                    stackgap           = stackgap - 1;
                    if (stackgap      <  1)
                    {
                        stackgap       = 1;
                    }
                }
                else if (modeflag     == MODE_BACKTRACE)
                {
                    btstart            = btstart - 16;
                    if (btstart       <  0)
                    {
                        btstart        = 48;
                    }
                }
                else if (modeflag     == MODE_INPPORTS)
                {
                    gamepad            = (gamepad - 1) % 4;
                    if (gamepad       <  0)
                    {
                        gamepad        = 3;
                    }
                }
                else if (modeflag     == MODE_MEMPORTS)
                {
                    cardstart          = cardstart - 16;
                    if (cardstart     <  0x30000000)
                    {
                        cardstart      = 0x3003FFF0;
                    }
                }
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // LEFT: recognize key release after being pressed
            //
            else if ((value           <= BUTTON_NOT_PRESSED) &&
                (decflag              == TRUE))
            {
                decflag                = FALSE;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // L: decrement content by 256
            //
            value                      = gamepad_button_l ();
            if ((value                >= BUTTON_IS_PRESSED) &&
                (decflag              == FALSE))
            {
                decflag                = 2;
                if (modeflag          == MODE_MEMORY)
                {
                    memstart           = memstart - 256;
                    if (memstart      <  0x00000000)
                    {
                        memstart       = 0x003FFF00;
                    }
                }
                else if (modeflag     == MODE_STACK)
                {
                    stackgap           = stackgap - 3;
                    if (stackgap      <  1)
                    {
                        stackgap       = 1;
                    }
                }
                else if (modeflag     == MODE_BACKTRACE)
                {
                    btstart            = 0;
                }
                else if (modeflag     == MODE_INPPORTS)
                {
                    gamepad            = 0;
                }
                else if (modeflag     == MODE_MEMPORTS)
                {
                    cardstart          = cardstart - 256;
                    if (cardstart     <  0x30000000)
                    {
                        cardstart      = 0x3003FF00;
                    }
                }
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // L: recognize key release after being pressed
            //
            else if ((value           <= BUTTON_NOT_PRESSED) &&
                (decflag              == 2))
            {
                decflag                = FALSE;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // RIGHT: increment content by 16
            //
            value                      = gamepad_right ();
            if ((value                >= BUTTON_IS_PRESSED) &&
                (incflag              == FALSE))
            {
                incflag                = 2;
                if (modeflag          == MODE_MEMORY)
                {
                    memstart           = memstart + 16;
                    if (memstart      >  0x003FFFF0)
                    {
                        memstart       = 0x00000000;
                    }
                }
                else if (modeflag     == MODE_STACK)
                {
                    stackgap           = stackgap + 1;
                    if (stackgap      >  18)
                    {
                        stackgap       = 18;
                    }
                }
                else if (modeflag     == MODE_BACKTRACE)
                {
                    btstart            = btstart  + 16;
                    if (btstart       >  63)
                    {
                        btstart        = 0;
                    }
                }
                else if (modeflag     == MODE_INPPORTS)
                {
                    gamepad            = (gamepad + 1) % 4;
                }
                else if (modeflag     == MODE_MEMPORTS)
                {
                    cardstart          = cardstart + 16;
                    if (cardstart     >  0x3003FFFF)
                    {
                        cardstart      = 0x30000000;
                    }
                }
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // RIGHT: recognize key release after being pressed
            //
            else if ((value           <= BUTTON_NOT_PRESSED) &&
                (incflag              == 2))
            {
                incflag                = FALSE;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // R: increment content by 256
            //
            value                      = gamepad_button_r ();
            if ((value                >= BUTTON_IS_PRESSED) &&
                (incflag              == FALSE))
            {
                incflag                = TRUE;
                if (modeflag          == MODE_MEMORY)
                {
                    memstart           = memstart + 256;
                    if (memstart      >  0x003FFF00)
                    {
                        memstart       = 0x00000000;
                    }
                }
                else if (modeflag     == MODE_STACK)
                {
                    stackgap           = stackgap + 3;
                    if (stackgap      >  18)
                    {
                        stackgap       = 18;
                    }
                }
                else if (modeflag     == MODE_BACKTRACE)
                {
                    btstart            = 48;
                }
                else if (modeflag     == MODE_INPPORTS)
                {
                    gamepad            = 3;
                }
                else if (modeflag     == MODE_MEMPORTS)
                {
                    cardstart          = cardstart + 256;
                    if (cardstart     >  0x3003FF00)
                    {
                        cardstart      = 0x30000000;
                    }
                }
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // R: recognize key release after being pressed
            //
            else if ((value           <= BUTTON_NOT_PRESSED) &&
                (incflag              == TRUE))
            {
                incflag                = FALSE;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // DOWN: perform single step execution of current instruction
            //
            value                      = gamepad_down ();
            if (value                 >= BUTTON_IS_PRESSED)
            {
                stepflag               = TRUE;
                if (codemode          == DEBUG_C)
                {
                    cstepflag          = TRUE; // short-circuit pressing of down
                }
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // DOWN: recognize key release after being pressed (framestop
            // is  what  gets   us  out  of  the  loop   to  execute  the
            // instruction).
            //
            else if ((value           <= BUTTON_NOT_PRESSED) &&
                     (stepflag        == TRUE))
            {
                framestop              = 0; // actually perform single step
            }                               // (by releasing down once pressed)

            ////////////////////////////////////////////////////////////////////////////
            //
            // START: remain  in debugger, but continue  normal execution
            // of CART instructions (occurs on release after press)
            //
            value                      = gamepad_button_start ();
            if ((value                >= BUTTON_IS_PRESSED) &&
                (continueflag         == CONTINUE_NONE))
            {
                continueflag           = CONTINUE_ENTRIGGER;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // START:  recognize key  release  after  being pressed  (the
            // setting  of   continueflag  to   2  allows   framestop  to
            // continually  be set  to  0, prevently  a  re-entry of  the
            // single-step  prompt). Note  that with  this, we  are still
            // in  the  debugging monitor,  and  can  break back  to  the
            // single-step prompt by pressing START again.
            //
            else if ((value           <= BUTTON_NOT_PRESSED) &&
                     (continueflag    == CONTINUE_ENTRIGGER))
            {
                continueflag           = CONTINUE_ENABLED;
                framestop              = 0;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // X:  exit from  the debugger,  resuming normal  unmonitored
            // execution of instructions (occurs on release after press)
            //
            value                      = gamepad_button_x ();
            if ((value                >= BUTTON_IS_PRESSED) &&
                (exitflag             == FALSE))
            {
                exitflag               = TRUE;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // X: recognize key release  after being pressed (the jumping
            // to offset  will break us  out of the  debugger environment
            // for the remainder of the session).
            //
            else if ((value           <= BUTTON_NOT_PRESSED) &&
                     (exitflag        == TRUE))
            {
                ////////////////////////////////////////////////////////////////////////
                //
                // Since  we are  escaping the  debugger, set  our stored
                // offset value  to some unique, impossible  value so any
                // error_handler()  call to  service system  errors won't
                // rewrite the value of the provided InstructionPointer:
                //
                code                   = (int *) ADDR_CART_OFFSET;
                *code                  = 0xDEADBEEF;         // why not?

                ////////////////////////////////////////////////////////////////////////
                //
                // generate ideal custom subroutine  to handle the escape
                // from the BIOS/debugger:
                //
                index                  = 0;
                code                   = (int *) ADDR_CUSTOM_ROUTINE;
                *(code+index++)        = 0x0A000000;         // JMP to immediate
                *(code+index++)        = (int) offset;       // immediate value
                *(code+index++)        = 0x00000000;         // HLT for safety

                ////////////////////////////////////////////////////////////////////////
                //
                // switch to CART context; no need to back up BIOS, since
                // the debugger is being exit
                //
                asm
                {
                    "MOV R0,                  [0x003FFFE3]"
                    "OUT GPU_ClearColor,      R0"
                    "MOV R0,                  [0x003FFFE4]"
                    "OUT GPU_MultiplyColor,   R0"
                    "MOV R0,                  [0x003FFFE5]"
                    "OUT GPU_ActiveBlending,  R0"
                    "MOV R0,                  [0x003FFFE6]"
                    "OUT GPU_SelectedTexture, R0"
                    "MOV R0,                  [0x003FFFE7]"
                    "OUT GPU_SelectedRegion,  R0"
                    "MOV R0,                  [0x003FFFE8]"
                    "OUT GPU_DrawingPointX,   R0"
                    "MOV R0,                  [0x003FFFE9]"
                    "OUT GPU_DrawingPointY,   R0"
                    "MOV R0,                  [0x003FFFEA]"
                    "OUT GPU_DrawingScaleX,   R0"
                    "MOV R0,                  [0x003FFFEB]"
                    "OUT GPU_DrawingScaleY,   R0"
                    "MOV R0,                  [0x003FFFEC]"
                    "OUT GPU_DrawingAngle,    R0"
                    "MOV R0,                  [0x003FFFED]"
                    "OUT INP_SelectedGamepad, R0"
                    "MOV R0,                  [0x003FFBA0]" // restore CART register
                    "MOV R1,                  [0x003FFBA1]" // restore CART register
                    "MOV R2,                  [0x003FFBA2]" // restore CART register
                    "MOV R3,                  [0x003FFBA3]" // restore CART register
                    "MOV R4,                  [0x003FFBA4]" // restore CART register
                    "MOV R5,                  [0x003FFBA5]" // restore CART register
                    "MOV R6,                  [0x003FFBA6]" // restore CART register
                    "MOV R7,                  [0x003FFBA7]" // restore CART register
                    "MOV R8,                  [0x003FFBA8]" // restore CART register
                    "MOV R9,                  [0x003FFBA9]" // restore CART register
                    "MOV R10,                 [0x003FFBAA]" // restore CART register
                    "MOV R11,                 [0x003FFBAB]" // restore CART register
                    "MOV R12,                 [0x003FFBAC]" // restore CART register
                    "MOV R13,                 [0x003FFBAD]" // restore CART register
                    "MOV R14,                 [0x003FFBAE]" // restore CART stack BP
                    "MOV R15,                 [0x003FFBAF]" // restore CART stack SP
                    "JMP 0x003FFFB0"                        // jump to custom routine
                }
            }

            end_frame ();
        } // end of single step loop

        instruction                    = *offset;
        immflag                        = instruction  & 0x02000000;
        srcreg                         = (instruction & 0x01E00000) >> 21;
        port                           = instruction  & 0x00003FFF;
        if (immflag                   >  0)
        {
            immediate                  = *(offset+1);
        }
        else
        {
            immediate                  = 0;
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // SANDBOXING:  to  preserve  debugger  monitoring  control,  any
        // branch instruction must be  intercepted and emulated. They are
        // never run in the custom routine, as branching would escape the
        // BIOS.
        //
        switch ((instruction & 0xFC000000) >> 26)
        {
            ////////////////////////////////////////////////////////////////////////////
            //
            // implement JMP emulation
            //
            case OPCODE_JMP:
                if (immflag           >  0)                 // if immediate bit is set
                {
                    offset             = (int *) immediate; // addr of first CART word
                }
                else
                {
                    pos                = (instruction & 0x01E00000) >> 21;
                    code               = (int *) (ADDR_CART_REGISTERS + pos);
                    offset             = (int *) *code;
                }
                continue;

            ////////////////////////////////////////////////////////////////////////////
            //
            // implement CALL  emulation: among the more  detailed of the
            // branch instructions  to emulate; since it  PUSHes onto the
            // stack, THAT  functionality also needs to  be performed, in
            // lieu of actually executing the native instruction
            //
            // Take note of the exceptional pointer scheme utilized
            //
            case OPCODE_CALL:
                ////////////////////////////////////////////////////////////////////////
                //
                // CART SP, stored in RAM
                //
                code                   = (int *) (ADDR_CART_REGISTERS + 15);
                *code                  = *code - 1; // CART SP = CART SP - 1 (pushing)

                ////////////////////////////////////////////////////////////////////////
                //
                // The trickiest,  most indirect part: to  accomplish the
                // PUSH, the address  needs to be placed  onto the stack-
                // but specifically the CART memory backup of SP.
                //
                mem                    = (int **) &(*code);
                **mem                  = (int) offset; // **mem is actual data storage

                if (immflag           >  0)
                {
                    offset             = (int *) immediate; // addr of CART word
                }
                else
                {
                    pos                = (instruction & 0x01E00000) >> 21;
                    code               = (int *) (ADDR_CART_REGISTERS + pos);
                    offset             = (int *) *code;
                }
                backtrace[btrace]      = (int) offset;
                btrace                 = btrace + 1;
                continue;

            ////////////////////////////////////////////////////////////////////////////
            //
            // implement RET  emulation: among  the more detailed  of the
            // branch instructions  to emulate.  Like CALL, which  has to
            // perform a  stack operation (PUSH),  here a POP is  what is
            // needing to be done.
            //
            case OPCODE_RET:
                ////////////////////////////////////////////////////////////////////////
                //
                // CART SP, stored in RAM
                //
                btrace                 = btrace - 1;
                backtrace[btrace]      = -1;
                code                   = (int *) (ADDR_CART_REGISTERS + 15);
                mem                    = (int **) &(*code);
                offset                 = (int *) **mem;
                *code                  = *code + 1; // CART SP = CART SP + 1 (popping)
                immflag                = (*offset) & 0x02000000;
                offset                 = offset + 1;
                if (immflag           >  0)
                {
                    offset             = offset + 1;
                }
                continue;

            case OPCODE_JT:
                dstreg                 = (instruction & 0x01E00000) >> 21;
                code                   = (int *) (ADDR_CART_REGISTERS + dstreg);
                if (*code             == 0) // false
                {
                    offset             = offset + 1;
                    if (immflag       >  0)
                    {
                        offset         = offset + 1;
                    }
                }
                else // true (we're jumping)
                {
                    if (immflag       >  0)
                    {
                        offset         = (int *) immediate;
                    }
                    else
                    {
                        srcreg         = (instruction & 0x01E00000) >> 21;
                        code           = (int *) (ADDR_CART_REGISTERS + srcreg);
                        offset         = (int *) *code;
                    }
                }
                continue;

            case OPCODE_JF:
                dstreg                 = (instruction & 0x01E00000) >> 21;
                code                   = (int *) (ADDR_CART_REGISTERS + dstreg);
                if (*code             != 0) // true (we're not jumping)
                {
                    offset             = offset + 1;
                    if (immflag       >  0)
                    {
                        offset         = offset + 1;
                    }
                }
                else // false (we're jumping)
                {
                    if (immflag       >  0)
                    {
                        offset         = (int *) immediate;
                    }
                    else
                    {
                        srcreg         = (instruction & 0x01E00000) >> 21;
                        code           = (int *) (ADDR_CART_REGISTERS + srcreg);
                        offset         = (int *) *code;
                    }
                }
                continue;

            ////////////////////////////////////////////////////////////////////////////
            //
            // WAIT: in general, the expectation is after a WAIT there is
            // a decent chance of a clear screen and then various  redraw
            // events, so if we look out for a WAIT, we can set a flag to
            // use with tracking drawing events
            //
            case OPCODE_WAIT:
                waitflag               = TRUE;
                break;

            ////////////////////////////////////////////////////////////////////////////
            //
            // OUT: be on  the lookout for clear  screen instances, which
            // will be  tied to  `clearflag` and  the continue  mode view
            // rendering.
            //
            // The OUT instruction  will be performed as  normal, we just
            // make  a quick  pit stop  here  to check  for the  specific
            // transaction, set the flag, then be back on our way.
            //
//| `0x10` | `GPUCommand_ClearScreen`          | clear screen using current color    |
//| `0x11` | `GPUCommand_DrawRegion`           | draw region: rotation off, zoom off |
//| `0x12` | `GPUCommand_DrawRegionZoomed`     | draw region: rotation off, zoom on  |
//| `0x13` | `GPUCommand_DrawRegionRotated`    | draw region: rotation on , zoom off |
//| `0x14` | `GPUCommand_DrawRegionRotozoomed` | draw region: rotation on , zoom on  |
            case OPCODE_OUT:
                if (port              == 0x200)    // GPU_Command
                {
                    if (immflag       >  0)        // data is immediate
                    {
                        value          = immediate;
                    }
                    else
                    {
                        asm
                        {
                            "PUSH R0"
                            "PUSH R1"
                            "MOV  R0,      0x003FFBA0"
                            "MOV  R1,      {srcreg}"
                            "IADD R0,      R1"
                            "MOV  R0,      [R0]"
                            "MOV  {value}, R0"
                            "POP  R1"
                            "POP  R0"
                        }
                    }

                    if (value         == 0x10)     // GPUCommand_ClearScreen
                    {
                        clearflag      = 1;

                        asm
                        {
                            "PUSH R0"
                            "MOV  R0,           0xDEADBEEF"
                            "MOV  [0x003FFFE2], R0"
                            "POP  R0"
                        }
                    }
                }
                break;

            default:
                break;
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // point  the  'code' pointer  at  the  starting address  of  our
        // dynamic subroutine
        //
        // For reference:
        //
        //   0x003FFFB0 start of dynamic subroutine (50 total words)
        //     . . .
        //   0x003FFFE2 end of dynamic subroutine
        //
        // The idea is to pack  the instructions into a specific location
        // in memory (RAM), in known  consecutive locations, then to CALL
        // it like  a subroutine, run  the desired instruction,  then RET
        // from it.
        //
        // 0x003FFFB0 start of dynamic subroutine (64 total words)
        //
        code                           = (int *) ADDR_CUSTOM_ROUTINE;  // (BP+1)

        ////////////////////////////////////////////////////////////////////////////////
        //
        // GENERATE THE  IDEAL SUBROUTINE:  knowing that the  compiler is
        // very stack- centric, it will generally use as few registers as
        // possible.
        //
        // We will  make use of  R10 (after backing  it up onto  the CART
        // stack) to  obtain the needed  "return" value, collected  in an
        // earlier assembly  sequence, as essentially a  "hack" giving us
        // access to the InstructionPointer.
        //
        // If  we keep  everything  stack and  memory-bound, that  should
        // mitigate  data hazards.  As  the debugger,  we  don't want  to
        // interfere with the normal execution of code.
        //
        // That  said, we  need  to  be mindful  we  are maintaining  TWO
        // different  stacks: the  DEBUGGER  stack, and  the CART  stack.
        // Whenever we  hop into this  custom subroutine, we'll  swap out
        // BP/SP with the CART stack values, execute the instruction then
        // store the CART register changes, revert back to BIOS mode, and
        // that is exactly what happens below (see SWITCHING CONTEXTS)
        //
        index                          = 0;
        *(code+index++)                = instruction; // instruction to process
        if (immflag                   >  0)
        {
            *(code+index++)            = immediate;   // immediate value
        }
        *(code+index++)                = 0x55400000;  // PUSH R10
        *(code+index++)                = 0x4F408000;  // MOV R10, [0x003FFFEF]
        *(code+index++)                = 0x003FFFEF;  // immediate data (retaddr)
        *(code+index++)                = 0x09400000;  // JMP R10
        *(code+index++)                = 0x00000000;  // HLT

        ////////////////////////////////////////////////////////////////////////////////
        //
        // SWITCHING CONTEXTS: BIOS -> CART
        //
        // pre-custom routine, back up BIOS environment and load CART environment
        //
        asm
        {
            "MOV [0x003FFFF0],        R0"  // back up BIOS to RAM
            "MOV [0x003FFFF1],        R1"  // back up BIOS to RAM
            "MOV [0x003FFFF2],        R2"  // back up BIOS to RAM
            "MOV [0x003FFFF3],        R3"  // back up BIOS to RAM
            "MOV [0x003FFFF4],        R4"  // back up BIOS to RAM
            "MOV [0x003FFFF5],        R5"  // back up BIOS to RAM
            "MOV [0x003FFFF6],        R6"  // back up BIOS to RAM
            "MOV [0x003FFFF7],        R7"  // back up BIOS to RAM
            "MOV [0x003FFFF8],        R8"  // back up BIOS to RAM
            "MOV [0x003FFFF9],        R9"  // back up BIOS to RAM
            "MOV [0x003FFFFA],        R10" // back up BIOS to RAM
            "MOV [0x003FFFFB],        R11" // back up BIOS to RAM
            "MOV [0x003FFFFC],        R12" // back up BIOS to RAM
            "MOV [0x003FFFFD],        R13" // back up BIOS to RAM
            "MOV [0x003FFFFE],        R14" // back up BIOS to RAM
            "MOV [0x003FFFFF],        R15" // back up BIOS to RAM
            "MOV R0,                  _CUSTOM_RET"  // grab returning offset
            "MOV [0x003FFFEF],        R0"           // place it in memory
            "MOV R0,                  [0x003FFFE3]" // restore CART GPU ports
            "OUT GPU_ClearColor,      R0"
            "MOV R0,                  [0x003FFFE4]"
            "OUT GPU_MultiplyColor,   R0"
            "MOV R0,                  [0x003FFFE5]"
            "OUT GPU_ActiveBlending,  R0"
            "MOV R0,                  [0x003FFFE6]"
            "OUT GPU_SelectedTexture, R0"
            "MOV R0,                  [0x003FFFE7]"
            "OUT GPU_SelectedRegion,  R0"
            "MOV R0,                  [0x003FFFE8]"
            "OUT GPU_DrawingPointX,   R0"
            "MOV R0,                  [0x003FFFE9]"
            "OUT GPU_DrawingPointY,   R0"
            "MOV R0,                  [0x003FFFEA]"
            "OUT GPU_DrawingScaleX,   R0"
            "MOV R0,                  [0x003FFFEB]"
            "OUT GPU_DrawingScaleY,   R0"
            "MOV R0,                  [0x003FFFEC]"
            "OUT GPU_DrawingAngle,    R0"
            "MOV R0,                  [0x003FFFED]"
            "OUT INP_SelectedGamepad, R0"
            "MOV R0,                  [0x003FFFED]" // grab CART gamepad
            "OUT INP_SelectedGamepad, R0"           // restore CART gamepad
            "MOV R0,                  [0x003FFBA0]" // restore CART register
            "MOV R1,                  [0x003FFBA1]" // restore CART register
            "MOV R2,                  [0x003FFBA2]" // restore CART register
            "MOV R3,                  [0x003FFBA3]" // restore CART register
            "MOV R4,                  [0x003FFBA4]" // restore CART register
            "MOV R5,                  [0x003FFBA5]" // restore CART register
            "MOV R6,                  [0x003FFBA6]" // restore CART register
            "MOV R7,                  [0x003FFBA7]" // restore CART register
            "MOV R8,                  [0x003FFBA8]" // restore CART register
            "MOV R9,                  [0x003FFBA9]" // restore CART register
            "MOV R10,                 [0x003FFBAA]" // restore CART register
            "MOV R11,                 [0x003FFBAB]" // restore CART register
            "MOV R12,                 [0x003FFBAC]" // restore CART register
            "MOV R13,                 [0x003FFBAD]" // restore CART register
            "MOV R14,                 [0x003FFBAE]" // restore CART stack BP
            "MOV R15,                 [0x003FFBAF]" // restore CART stack SP
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Thinking  on this  situation some  more,  with the  help of  a
        // well-placed label,  we can  have the  assembler solve  all our
        // offset  problems: we'll  let it  translate the  offset of  our
        // label, allowing  us to  simply place  the desired  offset into
        // memory at the intended location for our custom routine to grab
        // it, no finangling on our part necessary. Here is the old code,
        // preserved due to  the awesome hack employed to  give us access
        // to the InstructionPointer (IP) register:
        //
        //  "CALL _GET_IP_REG"      // offset jujitsu being  done at  the
        //  "JMP  _AFTER_GETTING"   // last moment we are  still in  BIOS
        //  "_GET_IP_REG:"          // mode, before switching to the CART
        //  "MOV R0, [SP]"          // grab our current location (IP)  in
        //  "MOV [0x003FFFEF], R0"  // stack, save it to RAM  in a  known
        //  "RET"                   // location to grab later
        //  "_AFTER_GETTING:"
        //
        ////////////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////////////
        //
        // "CALL" IDEAL ROUTINE: With our subroutine generated in memory,
        // we can now CALL it to run it through.
        //
        // As subroutine  instruction processing is  sequential, starting
        // at  the very  first address  and proceeding  downward (barring
        // any  branching),  it will  "just  work",  and  we can  do  any
        // post-processing as needed.
        //
        // The  start of  the subroutine  is  the address  stored in  the
        // 'code' integer pointer. So, JMP to it:
        //
        asm
        {
            "JMP   0x003FFFB0"
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // SWITCHING CONTEXTS: CART -> BIOS
        //
        // post-custom  routine,  back  up  CART  and  restore  the  BIOS
        // environment
        //
        asm
        {
            "_CUSTOM_RET:"
            "POP R10"               // R10 back to last legit CART state
            "MOV [0x003FFBA0], R0"  // back up CART to RAM
            "MOV [0x003FFBA1], R1"  // back up CART to RAM
            "MOV [0x003FFBA2], R2"  // back up CART to RAM
            "MOV [0x003FFBA3], R3"  // back up CART to RAM
            "MOV [0x003FFBA4], R4"  // back up CART to RAM
            "MOV [0x003FFBA5], R5"  // back up CART to RAM
            "MOV [0x003FFBA6], R6"  // back up CART to RAM
            "MOV [0x003FFBA7], R7"  // back up CART to RAM
            "MOV [0x003FFBA8], R8"  // back up CART to RAM
            "MOV [0x003FFBA9], R9"  // back up CART to RAM
            "MOV [0x003FFBAA], R10" // back up CART to RAM
            "MOV [0x003FFBAB], R11" // back up CART to RAM
            "MOV [0x003FFBAC], R12" // back up CART to RAM
            "MOV [0x003FFBAD], R13" // back up CART to RAM
            "MOV [0x003FFBAE], R14" // back up CART to RAM
            "MOV [0x003FFBAF], R15" // back up CART to RAM
            "IN  R0,           GPU_ClearColor" // back up CART GPU ports
            "MOV [0x003FFFE3], R0"
            "IN  R0,           GPU_MultiplyColor"
            "MOV [0x003FFFE4], R0"
            "IN  R0,           GPU_ActiveBlending"
            "MOV [0x003FFFE5], R0"
            "IN  R0,           GPU_SelectedTexture"
            "MOV [0x003FFFE6], R0"
            "IN  R0,           GPU_SelectedRegion"
            "MOV [0x003FFFE7], R0"
            "IN  R0,           GPU_DrawingPointX"
            "MOV [0x003FFFE8], R0"
            "IN  R0,           GPU_DrawingPointY"
            "MOV [0x003FFFE9], R0"
            "IN  R0,           GPU_DrawingScaleX"
            "MOV [0x003FFFEA], R0"
            "IN  R0,           GPU_DrawingScaleY"
            "MOV [0x003FFFEB], R0"
            "IN  R0,           GPU_DrawingAngle"
            "MOV [0x003FFFEC], R0"
            "IN  R0,           INP_SelectedGamepad" // back up current CART gamepad
            "MOV [0x003FFFED], R0"        // back up current CART gamepad
            "MOV R0,           [0x003FFFF0]" // restore BIOS register
            "MOV R1,           [0x003FFFF1]" // restore BIOS register
            "MOV R2,           [0x003FFFF2]" // restore BIOS register
            "MOV R3,           [0x003FFFF3]" // restore BIOS register
            "MOV R4,           [0x003FFFF4]" // restore BIOS register
            "MOV R5,           [0x003FFFF5]" // restore BIOS register
            "MOV R6,           [0x003FFFF6]" // restore BIOS register
            "MOV R7,           [0x003FFFF7]" // restore BIOS register
            "MOV R8,           [0x003FFFF8]" // restore BIOS register
            "MOV R9,           [0x003FFFF9]" // restore BIOS register
            "MOV R10,          [0x003FFFFA]" // restore BIOS register
            "MOV R11,          [0x003FFFFB]" // restore BIOS register
            "MOV R12,          [0x003FFFFC]" // restore BIOS register
            "MOV R13,          [0x003FFFFD]" // restore BIOS register
            "MOV R14,          [0x003FFFFE]" // restore BIOS stack BP
            "MOV R15,          [0x003FFFFF]" // restore BIOS stack SP
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // restore BIOS gamepad
        //
        select_gamepad (DEBUG_GAMEPAD);

        ////////////////////////////////////////////////////////////////////////////////
        //
        // advance CART "program counter"` by 1 (+ another 1 if immediate
        // data is present)
        //
        offset                         = offset + 1;
        if (immflag                   >  0)
        {
            offset                     = offset + 1;
        }
    }

    asm { "hlt" }
}
