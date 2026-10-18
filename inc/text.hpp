#pragma once
////////////////////////////////////////////////////////////////////////////////////////
//
// text.hpp - a small string builder
//
// The original debugger drew every piece of a decoded instruction with its own
// print_at() call and a hand-tracked x offset. Building the whole line in a
// buffer first and drawing it once is shorter, easier to read, and cuts the
// number of GPU calls (and function calls) per line to a fraction.
//
// There are no globals in a BIOS (see README), so the owner supplies the buffer.
//
////////////////////////////////////////////////////////////////////////////////////////
class Text
{
    int  *buffer;
    int   length;
    int   mark;      // where the immediate value starts (-1: none)

    public:
        // start an empty string in the given storage
        void init (int *storage)
        {
            buffer    = storage;
            length    = 0;
            mark      = -1;
            buffer[0] = 0;
        }

        // the string built so far
        int *text ()
        {
            return buffer;
        }

        // append one character
        void add (int character)
        {
            buffer[length] = character;
            length         = length + 1;
            buffer[length] = 0;
        }

        // append a zero-terminated string
        void str (int *string)
        {
            while (*string)
            {
                add (*string);
                string     = string + 1;
            }
        }

        // append `count` characters of a table of fixed-width strings
        void field (int *table, int index, int width)
        {
            int  position  = index * width;
            int  end       = position + width;

            while (position < end)
            {
                add (table[position]);
                position   = position + 1;
            }
        }

        // append the low `digits` hex digits of a value
        void hex (int value, int digits)
        {
            int  shift     = (digits - 1) * 4;
            int  nibble    = 0;

            while (shift  >= 0)
            {
                nibble     = (value >> shift) & 15;
                if (nibble >  9)
                {
                    nibble = nibble + 7;
                }
                add (nibble + 0x30);
                shift      = shift - 4;
            }
        }

        // append "0x" and 8 hex digits
        void hex32 (int value)
        {
            add ('0');
            add ('x');
            hex (value, 8);
        }

        // append an instruction's immediate value in hex, noting where it starts
        // (the screen draws it in the debugger's narrowed digits)
        void imm (int value)
        {
            mark       = length;
            hex32 (value);
        }

        // where the immediate starts, or -1
        int mark_position ()
        {
            return mark;
        }

        // append a signed decimal number
        void dec (int value)
        {
            int  digits[12];

            itoa (value, digits, 10);
            str  (digits);
        }

        // append a register name: R0 ... R15
        void reg (int number)
        {
            add ('R');
            dec (number);
        }

        // append a float, given as the raw 32 bits it is stored in
        void flt (int bits)
        {
            int    digits[33];
            float *number  = (float *) &bits;

            ftoa (*number, digits);
            str  (digits);
        }
};
