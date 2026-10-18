#pragma once
////////////////////////////////////////////////////////////////////////////////////////
//
// debugger.hpp - the debugger itself
//
// A BIOS has no safe place for global variables (they would sit at the start of
// RAM, where the CART expects its own), so everything the debugger remembers
// lives in this object, which main() keeps on the debugger stack. The CART's
// registers and the few other values both sides must agree on sit at fixed
// addresses at the top of RAM (see the memory map in defines.hpp).
//
// How a CART instruction runs. The debugger never lets the CART execute by
// itself. To run instructions it copies them, word for word, into a small
// routine in RAM, adds a few words that jump back, loads the CART registers and
// jumps in; when the routine jumps back, the registers are saved again. Anything
// that would move the instruction pointer (JMP, CALL, RET, JT, JF) is never run:
// the debugger changes its own "program counter" (`pc`) instead, so the CART can
// never get away from it.
//
// Run one at a time, that is the single-step mode. In CONTINUATION mode the
// instructions between two branches are run together, which is what makes it
// fast: one switch of context per run instead of one per instruction.
//
////////////////////////////////////////////////////////////////////////////////////////
class Debugger
{
    public:
        ////////////////////////////////////////////////////////////////////////////////
        //
        // state
        //
        int    *pc;                       // CART address of the next instruction
        int     io_live;                  // TRUE: the hardware holds the CART's GPU
                                          // state, FALSE: it holds the BIOS's
        int     clear_seen;               // the CART cleared the screen
        int     mode;                     // content view (MODE_*)
        int     continuing;               // CONTINUATION mode
        int     halted;                   // the CART is at a HLT
        int     rom_end;                  // first address past the CART program
        int     view[NUM_MODES];          // view parameter for each MODE_*
        int     history_store[HISTORY_SIZE];
        int     trace_store[TRACE_SIZE];
        Ring    history;                  // addresses of the instructions run last
        Ring    trace;                    // call targets, newest last
        Input   input;                    // gamepad and keyboard

        ////////////////////////////////////////////////////////////////////////////////
        //
        // debugger.cpp: the interface
        //
        void    init            (void);
        void    run             (void);
        void    interact        (void);
        void    run_continued   (void);
        void    navigate        (int nav);
        void    adjust          (int direction, int big);
        void    escape          (void);
        void    acquire         (void);
        void    release         (void);
        void    draw_history    (void);
        int     draw_instruction(int *address, int y, int color);

        ////////////////////////////////////////////////////////////////////////////////
        //
        // exec.cpp: running the CART
        //
        void    execute         (int limit);
        void    emulate         (int instruction, int opcode);
        void    run_routine     (void);
        void    fault           (int code, int instruction, int immediate);
        int     readable        (int address);

        ////////////////////////////////////////////////////////////////////////////////
        //
        // views.cpp: the resource views
        //
        void    draw_view       (void);
        void    view_registers  (void);
        void    view_ram        (void);
        void    view_stack      (void);
        void    view_backtrace  (void);
        void    view_timer      (void);
        void    view_rng        (void);
        void    view_gpu        (void);
        void    view_spu        (void);
        void    view_input      (void);
        void    view_cartridge  (void);
        void    view_memcard    (void);
};
