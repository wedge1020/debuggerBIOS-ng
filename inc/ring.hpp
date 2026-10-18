#pragma once
////////////////////////////////////////////////////////////////////////////////////////
//
// ring.hpp - fixed size ring buffer
//
// The debugger needs two rolling windows: the last few instructions executed
// (the history shown while single-stepping) and the live CALL chain (the
// backtrace). Both used to be plain arrays, with the history shifted down one
// entry at a time for every instruction executed and the backtrace cleared and
// indexed unchecked. A ring replaces all that: pushing is a store and two
// integer operations, no matter how big the window is, and the oldest entry
// simply falls off the end.
//
// The ring does not own its storage; the owner hands it a buffer whose size is a
// power of two (so the wrap is a mask, not a division).
//
////////////////////////////////////////////////////////////////////////////////////////
class Ring
{
    public:
        // The fields are public on purpose: the executor, which pushes one entry
        // per CART instruction, does so with plain stores on its hot path.
        int  *store;     // storage, `mask + 1` words
        int   mask;      // size - 1
        int   head;      // where the next push goes
        int   used;      // entries held (never more than the size)

        void init (int *buffer, int size)
        {
            store = buffer;
            mask  = size - 1;
            head  = 0;
            used  = 0;
        }

        // forget everything
        void clear ()
        {
            head  = 0;
            used  = 0;
        }

        // add an entry, dropping the oldest one if the ring is full
        void push (int value)
        {
            store[head] = value;
            head        = (head + 1) & mask;
            if (used   <= mask)
            {
                used    = used + 1;
            }
        }

        // remove and return the newest entry (-1 when there is none)
        int pop ()
        {
            if (used == 0)
            {
                return -1;
            }
            head        = (head - 1) & mask;
            used        = used - 1;
            return store[head];
        }

        // entries held
        int count ()
        {
            return used;
        }

        // size of the window
        int capacity ()
        {
            return mask + 1;
        }

        // entry by age: 0 is the oldest one held, count() - 1 the newest
        int get (int index)
        {
            return store[(head - used + index) & mask];
        }

        // the newest entry (-1 when there is none)
        int newest ()
        {
            if (used == 0)
            {
                return -1;
            }
            return store[(head - 1) & mask];
        }
};
