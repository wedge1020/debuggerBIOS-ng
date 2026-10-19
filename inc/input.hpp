#pragma once
////////////////////////////////////////////////////////////////////////////////////////
//
// input.hpp - the debugger's controls: gamepad and (optionally) a v32kbd keyboard
//
// The debugger sees the two devices as one set of eight controls (see CTRL_* in
// defines.hpp). What a control does depends on its kind:
//
//   ACTIONS - DOWN (step), START+X (enter/leave continuation), X (escape)
//
//       They fire when the control is RELEASED, and only if it was pressed
//       alone: no other control may go down while it is held. Press DOWN and
//       then bump LEFT, and nothing happens when DOWN comes up; the whole
//       chord is ignored until every control has been let go. A press that was
//       already in progress when the debugger started is ignored in the same
//       way. This is what keeps a clumsy hold on the pad from launching the CART
//       out of the debugger, or leaving continue mode by accident.
//
//       The one exception is the START+X combo, which toggles CONTINUATION mode.
//       Either control may go down first (or both in the same frame); nothing
//       else may join them; it fires when both have been released. START by
//       itself does nothing, so a stray START (the button that powered the
//       console on, a v32io keyboard Enter) can never start or stop the CART.
//       X by itself is still ESCAPE: it fires on release, unless START joined.
//
//   NAVIGATION - UP (cycle views), LEFT/RIGHT/L/R (adjust the view)
//
//       They act the moment they go down; LEFT, RIGHT, L and R repeat while
//       they stay down.
//
// Everything is sampled at most once per video frame: a v32kbd reports one key
// event per frame and holds it only until the next one.
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
// v32kbd key codes the debugger understands (the rest are ignored)
//
#define  KEY_UP                  1
#define  KEY_DOWN                2
#define  KEY_LEFT                3
#define  KEY_RIGHT               4
#define  KEY_TAB                 9
#define  KEY_ENTER               13
#define  KEY_ESCAPE              27
#define  KEY_PAGE_HIGH           30   // codepage prefixes: the next event is a
#define  KEY_PAGE_LOW            31   // character payload, not a key

////////////////////////////////////////////////////////////////////////////////////////
//
// v32kbd protocol, as packed from the gamepad controls (bit n = control n:
// Left, Right, Up, Down, Start, A, B, X, Y, L, R)
//
#define  KBD_STROBE              3    // Left | Right: alternates with each event
#define  KBD_PRESSED             4    // Up: the event is a key press
#define  KBD_CODE_SHIFT          4    // key code starts at Start (bit 4)
#define  KBD_CODE_MASK           127
#define  KBD_PAGE_TIMEOUT        30   // frames a codepage prefix waits for its payload

class Input
{
    public:
        int   held;            // controls down at the latest sample
        int   armed;           // action control that was pressed alone (0: none)
        int   dead;            // a chord happened: no action until all are released
        int   frame;           // frame of the latest sample (-1: none yet)
        int   nav;             // navigation controls that fired at the latest sample
        int   action;          // ACTION_* that fired at the latest sample
        int   age[NUM_CONTROLS];   // frames each control has been down
        int   due[NUM_CONTROLS];   // age at which a held control next repeats

        int   kbd_present;     // keyboard connected at the latest sample
        int   kbd_strobe;      // strobe side of the latest event seen
        int   kbd_page;        // codepage prefix waiting for its payload
        int   kbd_page_frame;  // frame the prefix arrived at
        int   kbd_held;        // controls held down by the keyboard

        void  init       (void);
        int   sample     (void);          // TRUE when a new frame was sampled

    private:
        int   read_pad   (void);
        int   read_kbd   (int now);
        int   key_control(int key);
};
