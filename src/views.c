////////////////////////////////////////////////////////////////////////////////////////
//
// views(): process the individual resource views
//
//void  views (int  modeflag, int *offset, int  memstart, int  stackgap, int  gamepad, int  cardstart, int *backtrace, int  btstart)
void  views (int  modeflag, int *offset, int *viewflags, int *backtrace)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Declare and initialize variables
    //
    float     fvalue               = 0.0;
    int [16]  data;
    int      *address              = NULL;
    int      *stack                = NULL;
    int       index                = 0;
    int       pos                  = 0;
    int       value                = 0;
    int       viewval              = 0;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Ensure the data array is cleared
    //
    for (index                     = 0;
         index                    <  16;
         index                     = index + 1)
    {
        data[index]                = 0;         // clear the data array
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // obtain the mode-specific view value
    //
    if ((modeflag                 >= MODE_NONE) &&
        (modeflag                 <  MAX_MODES))
    {
        viewval                    = *(viewflags+modeflag);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Process for the selected mode
    //
    switch (modeflag)
    {
        ////////////////////////////////////////////////////////////////////////////////
        //
        // register display: if enabled, the
        // CART register values will be displayed off to the right
        //
        case MODE_REGISTER:
            print_zoomed_at (490, 0, "register view", 1.0);
            select_region   (region_divider_h);
            draw_region_at  (490, 20);
            draw_region_at  (540, 20);

            for (index             = 0;
                 index            <  16;        // for all registers
                 index             = index + 1)
            {
                itoa (index, &data[1], 10);
                data[0]            = 'R';
                if (index         <  10)
                {
                    data[2]        = ':';
                }
                else
                {
                    data[3]        = ':';
                }
                print_at (490, 18 + (index * 18), data); 

                ////////////////////////////////////////////////////////////////////
                //
                // display CART register value (backed up in memory)
                //
                address            = (int *) ADDR_CART_REGISTERS;
                hexit_zoomed (540, 18 + (index * 18), *(address+index), 0.75);
            }

            print_at (490, 18 + (index * 18), "IP:");
            hexit_zoomed (540, 18 + (index * 18), (int) offset, 0.75);
            print_at (490, 18 + ((index+1) * 18), "IR:");
            value                  = *offset;
            hexit_zoomed (540, 18 + ((index+1) * 18), value, 0.75);
            if ((value & 0x02000000) >  0)
            {
                value                 = *(offset+1);
                print_at (490, 18 + ((index+2) * 18), "IV:");
                hexit_zoomed (540, 18 + ((index+2) * 18), value, 0.75);
            }
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // backtrace view: if selected, and the backtrace array populated,
        // display the current function call stack
        //
        case MODE_BACKTRACE:
            print_zoomed_at (464, 0, "backtrace", 1.0);
            select_region   (region_divider_h);
            draw_region_at  (464, 20);
            draw_region_at  (540, 20);

            pos                    = 20;
            for (index             = 15;
                 index            >= 0;
                 index             = index - 1)
            {
                if (-1            == *(backtrace+(viewval+index)))
                {
                    continue;
                }
                print_zoomed_at (464, pos, "[", 0.75);
                itoa            ((index + viewval), data, 10);
                print_zoomed_at (474, pos, data, 0.75);
                print_zoomed_at (494, pos, "]:", 0.75);
                hexit_zoomed    (524, pos, *(backtrace+(viewval+index)), 0.75);
                pos                = pos + 20;
            }
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // memory display: if selected, the CART memory values will be
        // displayed off to the right
        //
        case MODE_MEMORY:
            print_zoomed_at (464, 0, "memaddr   value", 1.0);
            select_region   (region_divider_h);
            draw_region_at  (464, 20);
            draw_region_at  (560, 20);

            for (index             = 0;
                 index            <  16;        // run from 0 to 15, for all registers
                 index             = index + 1)
            {
                ////////////////////////////////////////////////////////////////////////
                //
                // display address
                //
                hexit_zoomed    (464, (20+(index * 20)), (viewval+index), 0.75);
                print_zoomed_at (544, (20+(index * 20)), ":", 0.75); 

                ////////////////////////////////////////////////////////////////////////
                //
                // display value at address
                //
                address            = (int *) (value+index);
                hexit_zoomed (560, (20+(index * 20)), *(address), 0.75);
            }
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // MODE_STACK: if selected, the stack registers and memory that
        // immediate surrounds the stack will be displayed
        //
        case MODE_STACK:
            print_zoomed_at (464, 0, "stack     value", 1.0);
            select_region   (region_divider_h);
            draw_region_at  (464, 20);
            draw_region_at  (560, 20);

            ////////////////////////////////////////////////////////////////////////////
            //
            // obtain the CART stack pointer
            //
            print_at (464, 20, "SP:"); 
            asm
            {
                "PUSH R0"
                "MOV  R0,        [0x003FFBAF]"
                "MOV  {address}, R0"
                "POP  R0"
            }
            hexit_zoomed (560, 20, (int) address, 0.75);

            ////////////////////////////////////////////////////////////////////////////
            //
            // obtain the CART base pointer
            //
            asm
            {
                "PUSH R0"
                "MOV  R0,        [0x003FFBAE]"
                "MOV  {stack}, R0"
                "POP  R0"
            }

            pos                    = 20;
            for (index             = 0;
                 index            <= viewval;
                 index             = index + 1)
            {
                if ((0            <= (int) (address+index)) &&
                    (0x003FFFFF   >= (int) (address+index)) &&
                    (stack        >= (address+index)))
                {
                    pos            = pos + 16;
                    print_at     (464, pos, "[SP+"); 
                    itoa         (index, data, 10);
                    print_at     (504, pos, data);
                    print_at     (524, pos, "]:");
                    hexit_zoomed (560, pos, *(address+index), 0.75);
                }
                else
                {
                    break;
                }

                if (stack         == (address+index))
                {
                    break;
                }
            }

            pos                    = pos + 16;
            print_at     (464, pos, "BP:"); 
            hexit_zoomed (560, pos, (int) stack, 0.75);

            for (index             = 0;
                 index            <  18 - viewval;
                 index             = index + 1)
            {
                if ((0            <= (int) (stack+index)) &&
                    (0x003FFFFF   >= (int) (stack+index)) &&
                    (stack        >= (address+index)))
                {
                    pos            = pos + 16;
                    print_at     (464, pos, "[BP+"); 
                    itoa         (index, data, 10);
                    print_at     (504, pos, data);
                    print_at     (524, pos, "]:");
                    hexit_zoomed (560, pos, *(stack+index), 0.75);
                }
                else
                {
                    break;
                }
            }
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // MODE_TIMPORTS: if selected, TIM ports and their values will be
        // displayed
        //
        case MODE_TIMPORTS:

            print_zoomed_at (464, 0, "TIM port value", 1.0);
            select_region   (region_divider_h);
            draw_region_at  (464, 20);
            draw_region_at  (560, 20);

            print_at (464, 20, "Date:");
            asm
            {
                "PUSH R0"
                "IN   R0,      TIM_CurrentDate"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 16);
            print_at (560, 20, data);

            print_at (464, 40, "Time:");
            asm
            {
                "PUSH R0"
                "IN   R0,      TIM_CurrentTime"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 16);
            print_at (560, 40, data);

            print_at (464, 60, "Frame #:");
            asm
            {
                "PUSH R0"
                "IN   R0,      TIM_FrameCounter"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 60, data);

            print_at (464, 80, "Cycles:");
            asm
            {
                "PUSH R0"
                "IN   R0,      TIM_CycleCounter"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 80, data);
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // MODE_RNGPORTS: if selected, RNG port and its value will be
        // displayed
        //
        case MODE_RNGPORTS:

            print_zoomed_at (464, 0, "RNG port value", 1.0);
            select_region   (region_divider_h);
            draw_region_at  (464, 20);
            draw_region_at  (560, 20);

            print_at (464, 20, "RAND:");
            asm
            {
                "PUSH R0"
                "MOV  R0,      [0x003FFFE1]"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 20, data);
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // MODE_GPUPORTS: if selected, GPU ports and their values will be
        // displayed
        //
        case MODE_GPUPORTS:

            print_zoomed_at (464, 0, "GPU port value", 1.0);
            select_region   (region_divider_h);
            draw_region_at  (464, 20);
            draw_region_at  (560, 20);

            print_at (464, 20, "Clear:");
            asm
            {
                "PUSH R0"
                "MOV  R0,      [0x003FFFE3]"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 16);
            print_at (560, 20, data);

            print_at (464, 40, "Multiply:");
            asm
            {
                "PUSH R0"
                "MOV  R0,      [0x003FFFE4]"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 40, data);

            print_at (464, 60, "Blending:");
            asm
            {
                "PUSH R0"
                "MOV  R0,      [0x003FFFE5]"
                "MOV  {value}, R0"
                "POP  R0"
            }

            if (value       == 0x20)
            {
                print_at (560, 60, "alpha");
            }
            else if (value  == 0x21)
            {
                print_at (560, 60, "add");
            }
            else
            {
                print_at (560, 60, "subtract");
            }

            print_at (464, 80, "Texture:");
            asm
            {
                "PUSH R0"
                "MOV  R0,      [0x003FFFE6]"
                "OUT  GPU_SelectedTexture, R0"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 80, data);

            print_at (464, 100, "Region:");
            asm
            {
                "PUSH R0"
                "MOV  R0,      [0x003FFFE7]"
                "OUT  GPU_SelectedRegion, R0"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 100, data);

            print_at (464, 120, "DrawX:");
            asm
            {
                "PUSH R0"
                "MOV  R0,      [0x003FFFE8]"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 120, data);

            print_at (464, 140, "DrawY:");
            asm
            {
                "PUSH R0"
                "MOV  R0,      [0x003FFFE9]"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 140, data);

            print_at (464, 160, "ScaleX:");
            asm
            {
                "PUSH R0"
                "MOV  R0,       [0x003FFFEA]"
                "MOV  {fvalue}, R0"
                "POP  R0"
            }
            ftoa (fvalue, data);
            print_at (560, 160, data);

            print_at (464, 180, "ScaleY:");
            asm
            {
                "PUSH R0"
                "MOV  R0,       [0x003FFFEB]"
                "MOV  {fvalue}, R0"
                "POP  R0"
            }
            ftoa (fvalue, data);
            print_at (560, 180, data);

            print_at (464, 200, "Angle:");
            asm
            {
                "PUSH R0"
                "MOV  R0,       [0x003FFFEC]"
                "MOV  {fvalue}, R0"
                "POP  R0"
            }
            ftoa (fvalue, data);
            print_at (560, 200, data);

            print_at (464, 220, "MinX:");
            asm
            {
                "PUSH R0"
                "IN   R0,      GPU_RegionMinX"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 220, data);

            print_at (464, 240, "MinY:");
            asm
            {
                "PUSH R0"
                "IN   R0,      GPU_RegionMinY"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 240, data);

            print_at (464, 260, "MaxX:");
            asm
            {
                "PUSH R0"
                "IN   R0,      GPU_RegionMaxX"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 260, data);

            print_at (464, 280, "MaxY:");
            asm
            {
                "PUSH R0"
                "IN   R0,      GPU_RegionMaxY"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 280, data);

            print_at (464, 300, "HotspotX:");
            asm
            {
                "PUSH R0"
                "IN   R0,      GPU_RegionHotSpotX"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 300, data);

            print_at (464, 320, "HotspotY:");
            asm
            {
                "PUSH R0"
                "IN   R0,      GPU_RegionHotspotY"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 320, data);
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // MODE_SPUPORTS: if selected, SPU ports and their values will be
        // displayed
        //
        case MODE_SPUPORTS:
            print_zoomed_at (464, 0, "SPU port value", 1.0);
            select_region   (region_divider_h);
            draw_region_at  (464, 20);
            draw_region_at  (560, 20);

            print_at (464, 20, "Volume:");
            asm
            {
                "PUSH R0"
                "IN   R0,                  SPU_GlobalVolume"
                "MOV  {fvalue},            R0"
                "POP  R0"
            }
            ftoa (fvalue, data);
            print_at (560, 20, data);

            print_at (464, 40, "Sound #:");
            asm
            {
                "PUSH R0"
                "IN   R0,                  SPU_SelectedSound"
                "MOV  {value},             R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 40, data);

            print_at (464, 60, "Channel#:");
            asm
            {
                "PUSH R0"
                "IN   R0,                  SPU_SelectedChannel"
                "MOV  {value},             R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 60, data);

            print_at (464, 80, "SoundLen:");
            asm
            {
                "PUSH R0"
                "IN   R0,                  SPU_SoundLength"
                "MOV  {value},             R0"
                "POP  R0"
            }
            hexit_zoomed (560, 80, value, 0.75);

            print_at (464, 100, "PlayLoop:");
            asm
            {
                "PUSH R0"
                "IN   R0,                  SPU_SoundPlayWithLoop"
                "MOV  {value},             R0"
                "POP  R0"
            }
            
            if (value             == 0)
            {
                print_at (560, 100, "false");
            }
            else
            {
                print_at (560, 100, "true");
            }

            print_at (464, 120, "LoopStrt:");
            asm
            {
                "PUSH R0"
                "IN   R0,                  SPU_SoundLoopStart"
                "MOV  {value},             R0"
                "POP  R0"
            }
            hexit_zoomed (560, 120, value, 0.75);

            print_at (464, 140, "LoopEnd:");
            asm
            {
                "PUSH R0"
                "IN   R0,                  SPU_SoundLoopEnd"
                "MOV  {value},             R0"
                "POP  R0"
            }
            hexit_zoomed (560, 140, value, 0.75);

            print_at (464, 160, "ChanStat:");
            asm
            {
                "PUSH R0"
                "IN   R0,                  SPU_ChannelState"
                "MOV  {value},             R0"
                "POP  R0"
            }

            switch (value)
            {
                case 0x40: // SPUChannelState_Stopped
                    print_at (560, 160, "stopped");
                    break;

                case 0x41: // SPUChannelState_Paused
                    print_at (560, 160, "paused");
                    break;

                case 0x42: // SPUChannelState_Playing
                    print_at (560, 160, "playing");
                    break;
            }

            print_at (464, 180, "ChnSet:");
            asm
            {
                "PUSH R0"
                "IN   R0,                  SPU_ChannelAssignedSound"
                "MOV  {value},             R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 180, data);

            print_at (464, 200, "ChanVol:");
            asm
            {
                "PUSH R0"
                "IN   R0,                  SPU_ChannelVolume"
                "MOV  {fvalue},            R0"
                "POP  R0"
            }
            ftoa (fvalue, data);
            print_at (560, 200, data);

            print_at (464, 220, "ChanSpd:");
            asm
            {
                "PUSH R0"
                "IN   R0,                  SPU_ChannelSpeed"
                "MOV  {fvalue},            R0"
                "POP  R0"
            }
            ftoa (fvalue, data);
            print_at (560, 220, data);

            print_at (464, 240, "ChanLoop:");
            asm
            {
                "PUSH R0"
                "IN   R0,                  SPU_ChannelLoopEnabled"
                "MOV  {value},             R0"
                "POP  R0"
            }
            
            if (value             == 0)
            {
                print_at (560, 240, "false");
            }
            else
            {
                print_at (560, 240, "true");
            }

            print_at (464, 260, "ChanPos:");
            asm
            {
                "PUSH R0"
                "IN   R0,                  SPU_ChannelPosition"
                "MOV  {fvalue},            R0"
                "POP  R0"
            }
            ftoa (fvalue, data);
            print_at (560, 260, data);
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // MODE_INPPORTS: if selected, INP ports and their values will be
        // displayed
        //
        case MODE_INPPORTS:
            print_zoomed_at (464, 0, "INP port value", 1.0);
            select_region   (region_divider_h);
            draw_region_at  (464, 20);
            draw_region_at  (560, 20);

            print_at (464, 20, "Gamepad:");
            select_gamepad (viewval);
            itoa (viewval, data, 10);
            print_at (560, 20, data);

            print_zoomed_at (464, 40, "Connected:", 0.75);
            asm
            {
                "PUSH R0"
                "IN   R0,      INP_GamepadConnected"
                "MOV  {value}, R0"
                "POP  R0"
            }

            if (value             == 1)
            {
                print_at (560, 40, "true");
            }
            else
            {
                print_at (560, 40, "false");
            }

            print_at (464, 60, "Left:");
            asm
            {
                "PUSH R0"
                "IN   R0,      INP_GamepadLeft"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 60, data);

            print_at (464, 80, "Right:");
            asm
            {
                "PUSH R0"
                "IN   R0,      INP_GamepadRight"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 80, data);

            print_at (464, 100, "Up:");
            asm
            {
                "PUSH R0"
                "IN   R0,      INP_GamepadUp"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 100, data);

            print_at (464, 120, "Down:");
            asm
            {
                "PUSH R0"
                "IN   R0,      INP_GamepadDown"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 120, data);

            print_at (464, 140, "Start:");
            asm
            {
                "PUSH R0"
                "IN   R0,      INP_GamepadButtonStart"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 140, data);

            print_at (464, 160, "A:");
            asm
            {
                "PUSH R0"
                "IN   R0,      INP_GamepadButtonA"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 160, data);

            print_at (464, 180, "B:");
            asm
            {
                "PUSH R0"
                "IN   R0,      INP_GamepadButtonB"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 180, data);

            print_at (464, 200, "X:");
            asm
            {
                "PUSH R0"
                "IN   R0,      INP_GamepadButtonX"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 200, data);

            print_at (464, 220, "Y:");
            asm
            {
                "PUSH R0"
                "IN   R0,      INP_GamepadButtonY"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 220, data);

            print_at (464, 240, "L:");
            asm
            {
                "PUSH R0"
                "IN   R0,      INP_GamepadButtonL"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 240, data);

            print_at (464, 260, "R:");
            asm
            {
                "PUSH R0"
                "IN   R0,      INP_GamepadButtonR"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 260, data);

            select_gamepad (DEBUG_GAMEPAD);
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // MODE_CARPORTS: if selected, CAR ports and their values will be
        // displayed
        //
        case MODE_CARPORTS:
            print_zoomed_at (464, 0, "CAR port value", 1.0);
            select_region   (region_divider_h);
            draw_region_at  (464, 20);
            draw_region_at  (560, 20);

            print_at (464, 20, "Connect:");
            asm
            {
                "PUSH R0"
                "IN   R0,      CAR_Connected"
                "MOV  {value}, R0"
                "POP  R0"
            }

            if (value        == 0)
            {
                print_at (560, 20, "false");
            }
            else
            {
                print_at (560, 20, "true");
            }

            print_at (464, 40, "ROMSize:");
            asm
            {
                "PUSH R0"
                "IN   R0,      CAR_ProgramROMSize"
                "MOV  {value}, R0"
                "POP  R0"
            }
            hexit_zoomed (560, 40, value, 0.75);

            print_at (464, 60, "#VTEX:");
            asm
            {
                "PUSH R0"
                "IN   R0,      CAR_NumberOfTextures"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 60, data);

            print_at (464, 80, "#VSND:");
            asm
            {
                "PUSH R0"
                "IN   R0,      CAR_NumberOfSounds"
                "MOV  {value}, R0"
                "POP  R0"
            }
            itoa (value, data, 10);
            print_at (560, 80, data);
            break;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // MODE_MEMPORTS: if selected, the MEM port and connected MEMCard data
        // will be displayed, with similar memory-adjustment controls available
        //
        case MODE_MEMPORTS:
            print_zoomed_at (464, 0, "MEM port value", 1.0);
            select_region   (region_divider_h);
            draw_region_at  (464, 20);
            draw_region_at  (560, 20);

            asm
            {
                "PUSH R0"
                "IN   R0,      MEM_Connected"
                "MOV  {value}, R0"
                "POP  R0"
            }

            if (value             == 0)
            {
                print_at (464, 20, "Connect:");
                print_at (560, 20, "false");
            }
            else
            {
                address            = (int *) 0x30000000;
                print_at (464, 20, address); // display MEMC title

                for (index         = 0;
                     index        <  16;
                     index         = index + 1)
                {
                    ////////////////////////////////////////////////////////////////////
                    //
                    // display address
                    //
                    hexit_zoomed    (464, (40+(index * 20)), (viewval+index), 0.75);
                    print_zoomed_at (544, (40+(index * 20)), ":", 0.75); 

                    ////////////////////////////////////////////////////////////////////
                    //
                    // display value at address
                    //
                    address            = (int *) (viewval+index);
                    hexit_zoomed (560, (40+(index * 20)), *(address), 0.75);
                }
            }
            break;
    }
}
