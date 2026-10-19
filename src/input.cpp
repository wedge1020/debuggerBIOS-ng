////////////////////////////////////////////////////////////////////////////////////////
//
// input.cpp - the debugger's controls: gamepad and (optionally) a v32kbd keyboard
//
// See input.hpp for how the controls behave.
//
////////////////////////////////////////////////////////////////////////////////////////

#define  ACTION_CONTROLS         (CTRL_DOWN | CTRL_START | CTRL_X)
#define  COMBO_CONTROLS          (CTRL_START | CTRL_X)    // toggles CONTINUATION mode
#define  REPEAT_CONTROLS         (CTRL_LEFT | CTRL_RIGHT | CTRL_L | CTRL_R)
#define  NAVIGATION_CONTROLS     (CTRL_UP | REPEAT_CONTROLS)

////////////////////////////////////////////////////////////////////////////////////////
//
// read_raw(): the 11 controls of the selected gamepad, packed one bit each in the
// order of the console's ports: Left, Right, Up, Down, Start, A, B, X, Y, L, R
//
static int read_raw (void)
{
    int  raw     = 0;

    if (read_port (PORT_INP_LEFT)  > 0) { raw = raw | 0x001; }
    if (read_port (PORT_INP_RIGHT) > 0) { raw = raw | 0x002; }
    if (read_port (PORT_INP_UP)    > 0) { raw = raw | 0x004; }
    if (read_port (PORT_INP_DOWN)  > 0) { raw = raw | 0x008; }
    if (read_port (PORT_INP_START) > 0) { raw = raw | 0x010; }
    if (read_port (PORT_INP_A)     > 0) { raw = raw | 0x020; }
    if (read_port (PORT_INP_B)     > 0) { raw = raw | 0x040; }
    if (read_port (PORT_INP_X)     > 0) { raw = raw | 0x080; }
    if (read_port (PORT_INP_Y)     > 0) { raw = raw | 0x100; }
    if (read_port (PORT_INP_L)     > 0) { raw = raw | 0x200; }
    if (read_port (PORT_INP_R)     > 0) { raw = raw | 0x400; }

    return raw;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// init(): start with no control held. Anything that is held right now (the button
// that was used to power the console on, say) has to be let go before it can fire
// an action: `dead` stays set until a sample finds every control up.
//
void Input::init (void)
{
    int  index     = 0;
    int  previous  = 0;

    held           = 0;
    armed          = 0;
    dead           = TRUE;
    frame          = -1;
    nav            = 0;
    action         = ACTION_NONE;

    for (index     = 0;
         index    <  NUM_CONTROLS;
         index     = index + 1)
    {
        age[index] = 0;
        due[index] = 0;
    }

    kbd_present    = FALSE;
    kbd_strobe     = 0;
    kbd_page       = 0;
    kbd_page_frame = 0;
    kbd_held       = 0;

#if KEYBOARD_ENABLED
    // the device keeps its last event across a console reset: take the strobe
    // side it shows now as the starting point, so that old event is not read as
    // a new one
    previous       = *((int *) (ADDR_SHADOW + SHADOW_GAMEPAD));
    select_gamepad (KEYBOARD_GAMEPAD);
    if (gamepad_is_connected ())
    {
        kbd_present = TRUE;
        kbd_strobe  = read_raw () & KBD_STROBE;
    }
    select_gamepad (previous);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////
//
// read_pad(): the debugger gamepad's controls, as CTRL_* bits
//
int Input::read_pad (void)
{
    int  state     = 0;

    if (read_port (PORT_INP_UP)    > 0) { state = state | CTRL_UP;    }
    if (read_port (PORT_INP_DOWN)  > 0) { state = state | CTRL_DOWN;  }
    if (read_port (PORT_INP_LEFT)  > 0) { state = state | CTRL_LEFT;  }
    if (read_port (PORT_INP_RIGHT) > 0) { state = state | CTRL_RIGHT; }
    if (read_port (PORT_INP_START) > 0) { state = state | CTRL_START; }
    if (read_port (PORT_INP_X)     > 0) { state = state | CTRL_X;     }
    if (read_port (PORT_INP_L)     > 0) { state = state | CTRL_L;     }
    if (read_port (PORT_INP_R)     > 0) { state = state | CTRL_R;     }

    return state;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// key_control(): the debugger control a v32kbd key stands for (0: none)
//
//   arrows .......... UP DOWN LEFT RIGHT
//   Enter, c ........ START (with X: toggle continue)
//   Space ........... DOWN  (step)
//   Escape, x ....... X     (escape)
//   [ , ............. L
//   ] . ............. R
//
int Input::key_control (int key)
{
    switch (key)
    {
        case KEY_UP:
            return CTRL_UP;

        case KEY_DOWN:
        case ' ':
            return CTRL_DOWN;

        case KEY_LEFT:
            return CTRL_LEFT;

        case KEY_RIGHT:
            return CTRL_RIGHT;

        case KEY_ENTER:
        case 'c':
            return CTRL_START;

        case KEY_ESCAPE:
        case 'x':
            return CTRL_X;

        case '[':
        case ',':
            return CTRL_L;

        case ']':
        case '.':
            return CTRL_R;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// read_kbd(): follow the keyboard on its gamepad port, returning the debugger
// controls whose keys are down.
//
// The v32kbd shows its latest key event as packed gamepad controls: the strobe
// (Left or Right) alternates for every new event, Up says press (else release),
// and the key code sits in the controls from Start up. Characters that are not on
// a US keyboard arrive as a codepage prefix and then a payload event, which must
// not be mistaken for keys.
//
int Input::read_kbd (int now)
{
    int  raw       = 0;
    int  side      = 0;
    int  key       = 0;
    int  control   = 0;

    select_gamepad (KEYBOARD_GAMEPAD);

    if (!gamepad_is_connected ())
    {
        kbd_present = FALSE;
        kbd_held    = 0;
        kbd_page    = 0;
        return 0;
    }

    // just plugged in: it shows every control released whatever the device
    // really holds, so its history starts over
    if (!kbd_present)
    {
        kbd_present = TRUE;
        kbd_strobe  = 0;
        kbd_page    = 0;
        kbd_held    = 0;
    }

    raw             = read_raw ();
    side            = raw & KBD_STROBE;

    if ((side      != 0) &&
        (side      != kbd_strobe))
    {
        kbd_strobe  = side;
        key         = (raw >> KBD_CODE_SHIFT) & KBD_CODE_MASK;

        // a prefix is waiting: this event is its payload, whatever its code
        // (one that waited too long was lost with its payload: forget it)
        if (kbd_page != 0)
        {
            if ((now - kbd_page_frame) <= KBD_PAGE_TIMEOUT)
            {
                kbd_page = 0;
                return kbd_held;
            }
            kbd_page    = 0;
        }

        if ((key == KEY_PAGE_HIGH) ||
            (key == KEY_PAGE_LOW))
        {
            kbd_page       = key;
            kbd_page_frame = now;
        }

        else if (key > 0)
        {
            control        = key_control (key);
            if (control   != 0)
            {
                if ((raw & KBD_PRESSED) != 0)
                {
                    kbd_held = kbd_held | control;
                }
                else
                {
                    kbd_held = kbd_held & (~control);
                }
            }
        }
    }

    return kbd_held;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// sample(): look at the devices, once per frame at most. Afterwards `nav` holds
// the navigation controls that fired and `action` the action (if any) that was
// released cleanly.
//
int Input::sample (void)
{
    int  now       = get_frame_counter ();
    int  delta     = 1;
    int  previous  = 0;
    int  state     = 0;
    int  edge      = 0;
    int  extra     = 0;
    int  index     = 0;
    int  bit       = 1;

    // what fired at the previous sample has been acted on by now: it must not
    // be seen twice (leaving continuation mode for the single-step screen, in
    // the same frame, would otherwise start it all over again)
    nav            = 0;
    action         = ACTION_NONE;

    if (now       == frame)
    {
        return FALSE;
    }

    if (frame     >= 0)
    {
        delta      = now - frame;
    }
    frame          = now;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // read the devices; the CART's gamepad selection is put back as it was
    //
    previous       = *((int *) (ADDR_SHADOW + SHADOW_GAMEPAD));
    select_gamepad (DEBUG_GAMEPAD);
    state          = read_pad ();

#if KEYBOARD_ENABLED
    state          = state | read_kbd (now);
#endif

    select_gamepad (previous);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // navigation: fires when the control goes down, and for some of them again
    // at regular intervals once held for a while
    //
    edge           = state & (~held);
    nav            = 0;
    action         = ACTION_NONE;

    for (index     = 0;
         index    <  NUM_CONTROLS;
         index     = index + 1)
    {
        if ((state & bit) == 0)
        {
            age[index]      = 0;
        }

        else if ((edge & bit) != 0)
        {
            age[index]      = 0;
            due[index]      = NAV_REPEAT_DELAY;
            if ((bit & NAVIGATION_CONTROLS) != 0)
            {
                nav         = nav | bit;
            }
        }

        else
        {
            age[index]      = age[index] + delta;
            if (((bit & REPEAT_CONTROLS) != 0) &&
                (age[index] >= due[index]))
            {
                nav         = nav | bit;
                due[index]  = age[index] + NAV_REPEAT_RATE;
            }
        }

        bit                 = bit + bit;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // actions: armed by a press that happens alone, cancelled by any other control
    // going down meanwhile, fired by the release.
    //
    // The one company allowed is the START+X combo (toggles CONTINUATION mode): when
    // START is armed and X goes down (or the other way round) the armed set becomes
    // the combo, and it fires once every control of it has been released. START
    // alone does nothing, which keeps a stray START (a power-on button, a v32io
    // keyboard Enter) from toggling anything.
    //
    if (armed != 0)
    {
        extra               = state & (~armed);

        if (extra          != 0)
        {
            if (((armed & COMBO_CONTROLS) == armed) &&
                (armed != COMBO_CONTROLS)           &&
                (extra == (COMBO_CONTROLS ^ armed)))
            {
                armed       = COMBO_CONTROLS;
            }
            else
            {
                armed       = 0;
                dead        = TRUE;
            }
        }

        else if ((state & armed) == 0)
        {
            if (armed      == CTRL_DOWN)
            {
                action      = ACTION_STEP;
            }
            else if (armed == COMBO_CONTROLS)
            {
                action      = ACTION_CONTINUE;
            }
            else if (armed == CTRL_X)
            {
                action      = ACTION_ESCAPE;
            }
            armed           = 0;
        }
    }

    else if ((dead == FALSE) &&
             ((edge & ACTION_CONTROLS) != 0))
    {
        // pressed alone, and a single one: arm it; the combo may also arrive whole,
        // both controls in the same frame; any other company makes it a chord
        if ((state == edge) &&
            (((state & (state - 1)) == 0) ||
             (state == COMBO_CONTROLS)))
        {
            armed           = state;
        }
        else
        {
            dead            = TRUE;
        }
    }

    if (state      == 0)
    {
        dead                = FALSE;
    }

    held                    = state;
    return TRUE;
}
