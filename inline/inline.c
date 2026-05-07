#include "video.h"
#include "string.h"
#include "time.h"
#include "misc.h"

void error_handler ()
{
	int  value  = 0;
	value       = value + 2;
}

void main ()
{
    int      value   = 2;
    int      offset  = 0;
    int [10] string;

    asm
    {
        "PUSH R0"
        "_UNRELATED_LABEL:"
        "MOV   R0,           {value}"
        "IADD  R0,           R0"
        "MOV   R1,           _LABEL"
        "ISUB  R1,           R0"
        "MOV   {offset},     R1"
        "POP   R0"
        "JMP   R1"
        "MOV   [0x00000007], R0"
        "MOV   [0x00000008], R0"
        "MOV   [0x00000009], R0"
        "MOV   [0x0000000A], R0"
        "_LABEL:"
        "MOV   [0x0000000B], R0"
    }

    itoa (value, string, 10);
    print_at (0,  100,  "value: ");
    print_at (80, 100,  string);

    itoa (offset, string, 16);
    print_at (0,  120, "offset: ");
    print_at (80, 120, string);

	value           = 1;

    asm
    {
        "PUSH R0"
        "_UNRELATED_LABEL2:"
        "MOV   R0,           {value}"
        "IADD  R0,           R0"
        "MOV   R1,           _LABEL_BETA"
        "ISUB  R1,           R0"
        "MOV   {offset},     R1"
        "POP   R0"
        "JMP   R1"
        "MOV   R0,           [0x00000007]"
        "MOV   R0,           [0x00000008]"
        "MOV   R0,           [0x00000009]"
        "MOV   R0,           [0x0000000A]"
        "_LABEL_BETA:"
        "MOV   R0,           [0x0000000B]"
    }

    itoa (value, string, 10);
    print_at (0,  200,  "value: ");
    print_at (80, 200,  string);

    itoa (offset, string, 16);
    print_at (0,  220, "offset: ");
    print_at (80, 220, string);

    end_frame ();

    exit ();
}
