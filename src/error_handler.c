////////////////////////////////////////////////////////////////////////////////////////
//
// error_handler(): process and report the system error
//
// NOTE:  this is  the  error_handler() developed  during the  spring2025
// offering of  the CSCS2650  Computer Organization  course at  SUNY CCC;
// ported over to add to the rich functionality of debuggerBIOS. 
//
void  error_handler ()
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Declare but NOT INITIALIZED  variables: default compiler action is
    // use R0  for transactions,  which will  override the  critical data
    // that the system placed in R0 for reporting here.
    //
    int   error_code;
    int   instruction_pointer;
    int   instruction;
    int   immediate_value;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // save registers to variables
    //
    asm
    {
        "mov   {error_code},          R0"
        "mov   {instruction_pointer}, R1"
        "mov   {instruction},         R2"
        "mov   {immediate_value},     R3"
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // ensure everything gets drawn
    //
    end_frame ();

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // variables for instruction decoding
    //
    int      *addr;
    int [11]  data;
    int       immflag;
    int [15]  list;
    int       word;
    int       pos;
    int       index;
    int       value;
    int       x;
    int       y;
    int       min;
    int       max;
    int       section;
    int      *offset;
    
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // initialize memory type array with their identified names    
    //
    string_data [4] memtype            =
    {
        { "[RAM] " }, { "[BIOS]" }, { "[CART]" }, { "[CARD]" }
    };
    memtype[0].lower                   = 0x00000000;
    memtype[0].upper                   = 0x003FFFFF;
    memtype[1].lower                   = 0x10000000;
    memtype[1].upper                   = 0x100FFFFF;
    memtype[2].lower                   = 0x20000000;
    memtype[2].upper                   = 0x27FFFFFF;
    memtype[3].lower                   = 0x30000000;
    memtype[3].upper                   = 0x30003FFF;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // determine messages for this error code
    //
    int  *error_title                  = NULL;
    int  *error_message                = NULL;

    switch (error_code)
    {
        case ERROR_MEMORY_READ:
            error_title                = "ERROR: INVALID MEMORY READ";
            error_message              = "Program attempted to read from a memory address\n"
                                         "that does not exist or is in a write-only device.";
            break;

        case ERROR_MEMORY_WRITE:
            error_title                = "ERROR: INVALID MEMORY WRITE";
            error_message              = "Program attempted to write on a memory address\n"
                                         "that does not exist or is in a read-only device.";
            break;

        case ERROR_PORT_READ:
            error_title                = "ERROR: INVALID PORT READ";
            error_message              = "Program attempted to read from a port number\n"
                                         "that does not exist or is set as write-only.";
            break;

        case ERROR_PORT_WRITE:
            error_title                = "ERROR: INVALID PORT WRITE";
            error_message              = "Program attempted to write on a port number\n"
                                         "that does not exist or is set as read-only.";
            break;

        case ERROR_STACK_OVERFLOW:
            error_title                = "ERROR: STACK OVERFLOW";
            error_message              = "Program pushed too many values in the stack\n"
                                         "and available RAM memory was exhausted.";
            break;

        case ERROR_STACK_UNDERFLOW:
            error_title                = "ERROR: STACK UNDERFLOW";
            error_message              = "Program popped too many values from the stack\n"
                                         "and all data stored in stack was exhausted.";
            break;

        case ERROR_DIVISION:
            error_title                = "ERROR: DIVISION BY ZERO";
            error_message              = "Program attempted to perform a division or\n"
                                         "modulus operation where the divisor was zero.";
            break;

        case ERROR_ARC_COSINE:
            error_title                = "ERROR: ARC COSINE OUT OF RANGE";
            error_message              = "Program attempted to perform an arc cosine\n"
                                         "operation when the argument was not in [-1,+1].";
            break;

        case ERROR_ARC_TANGENT_2:
            error_title                = "ERROR: ARC TANGENT NOT DEFINED";
            error_message              = "Program attempted to perform an arc tangent\n"
                                         "operation when both of the arguments were 0.";
            break;

        case ERROR_LOGARITHM:
            error_title                = "ERROR: LOGARITHM OUT OF RANGE";
            error_message              = "Program attempted to perform a logarithm\n"
                                         "operation when the argument is not positive.";
            break;

        case ERROR_POWER:
            error_title                = "ERROR: POWER HAS NO REAL SOLUTION";
            error_message              = "Program attempted to perform a power operation\n"
                                         "when base was negative and exponent non integer.";
            break;

        case ERROR_UNKNOWN:
            error_title                = "UNKNOWN ERROR";
            error_message              = "Program caused a hardware error with an error\n"
                                         "code that was not recognized by the BIOS.";
            break;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // draw the basic screen
    //
    set_drawing_scale (1.0, 1.0);
    draw_message_screen (error_title, error_message);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // CART context: when debugging, the offending CART instruction will be
    // the first instruction in the custom routine. To make the output more
    // valuable, adjust instruction_pointer to point to the actual location
    // in CART memory
    //
    offset                             = (int *) 0x003FFFEE;
    //if (0x003FFFB2                    == (instruction_pointer & 0xFFFFFFF0))
    /*
    offset                             = instruction_pointer & 0xFFFFFFF0;
    if (offset                        == 0x003FFFB2)
    {
        asm
        {
            "push R0"
            "mov  R0,       [0x003FFFEE]"
            "mov  {offset}, R0"
            "pop  R0"
        }*/

        if (*offset                   != 0xDEADBEEF)
        {
            instruction_pointer        = (int) *offset;
        }
    //}

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // ID  memory page  of error:  ascertain and  display type  of memory
    // where error condition occurred
    //
    section                            = (instruction_pointer & 0x30000000) >> 28;
    print_at (579, 0, memtype[section].name);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // now print the related hex values
    //
    x                                  = 49;
    set_multiply_color (error_colors_values);
    print_hex_value (x, 160, "Instruction Pointer", instruction_pointer);
    print_hex_value (x, 180, "Instruction        ", instruction);
    print_hex_value (x, 200, "Immediate Value    ", immediate_value);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // REPORTING: do not do instruction dump for BIOS errors
    //
    if (section                       != 1)
    {
        ////////////////////////////////////////////////////////////////////////////////
        //
        // lookahead instruction logic: decode up to error instruction + 2
        // for display on screen
        //
        addr                           = (int *)(instruction_pointer);
        pos                            = 9;
        max                            = 5;
        while (pos                    <  (max * 3))
        {
            list[pos]                  = (int)(addr); // addr of +N instruction
            list[pos+1]                = *(addr);     // actual hex of +N instruction
            immflag                    = (*(addr) & 0x02000000) >> 25;
            if (immflag               == 1)
            {
                addr                   = addr + 1;
                list[pos+2]            = *(addr); // immediate value to +N instruction
            }
            else
            {
                list[pos+2]            = 0;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // lookahead upper bound check
            //
            if ((int)addr             == memtype[section].upper)
            {
                max                    = (pos + 3) / 3;
                break;
            }

            addr                       = addr + 1;
            pos                        = pos  + 3;
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // look back, get to instruction (ensure instruction/immediate
        // alignment is correct)
        //
        pos                            = 6;
        addr                           = (int *) 0x20000000;
        if (instruction_pointer       >  0x20000000)
        {
            immflag                    = (instruction & 0x02000000) >> 25;

            if (immflag               == 1)
            {
                addr                   = (int *)(instruction_pointer-2);
                list[pos+2]            = *(addr+1);
            }
            else
            {
                addr                   = (int *)(instruction_pointer-1);
                list[pos+2]            = 0;
            }

            list[pos]                  = (int)(addr);
            list[pos+1]                = instruction;
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // lower bound check
        //
        if ((int)addr                 == memtype[section].lower)
        {
            min                        = pos;
        }
        else
        {
            min                        = 0;
            for (index                 = 3;
                 index                >= min;
                 index                 = index - 3)
            {
                addr                   = addr - 1;
                immflag                = (*(addr) & 0x02000000) >> 25;
                if (immflag           == 1) // definitely cannot be an instruction
                {
                    list[index+2]      = *(addr);     // immediate value
                    addr               = addr - 1;
                    list[index]        = (int)(addr); // addr of instruction
                    list[index+1]      = *(addr);     // actual hex of instruction
                }

                else // could be an instruction
                {
                    addr               = addr - 1; // check the previous for immediate flag
                    immflag            = (*(addr) & 0x02000000) >> 25;
                    if (immflag       == 1)
                    {
                        addr           = addr - 1; // if so, check the one before that
                        immflag        = (*(addr) & 0x02000000) >> 25;
                        if (immflag   == 1)
                        {
                            addr       = addr + 2;
                        }
                        else
                        {
                            addr       = addr + 1;
                        }
                        list[index]    = (int)(addr); // addr of instruction
                        list[index+1]  = *(addr);     // actual hex of instruction
                        list[index+2]  = *(addr+1);   // immediate value
                    }        
                    else
                    {
                        addr           = addr + 1;
                        list[index]    = (int)(addr); // addr of instruction
                        list[index+1]  = *(addr);     // actual hex of instruction
                        list[index+2]  = 0;           // immediate value
                    }
                }

                ////////////////////////////////////////////////////////////////////////
                //
                // lower bound check
                //
                if ((int)addr         == memtype[section].lower)
                {
                    min                = index;
                    break;
                }
            }
        }

        y                              = 240;
        for (index                     = 0;
             index                    <  max;
             index                     = index + 1)
        {
            ////////////////////////////////////////////////////////////////////////////
            //
            // Lower bound check for memory access
            //
            if (list[(index*3)+0]     <  memtype[section].lower)
            {
                continue;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // highlight the current (error) instruction red, others in white
            //
            if (list[(index*3)+1]     == instruction)
            {
                set_multiply_color (color_red);
            }
            else
            {
                set_multiply_color (color_white);
            }

            x                          = 49;           
            hexit (x, y, list[(index*3)]);
            x                          = x    + (strlen (data) * 10);
            print_at (x, y, ":");
            x                          = x    + 20;

            word                       = list[(index*3)+1];
            value                      = list[(index*3)+2];
            decode (169, y, word, value);
        
            y                          = y    + 20;

            ////////////////////////////////////////////////////////////////////////////
            //
            // Upper bound check for memory access
            //
            if (list[(index*3)+0]     == memtype[section].upper)
            {
                break;
            }
        }
    }

    print_at (579, 339, "[HALT]");

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // stop any sound
    //
    stop_all_channels ();
}
