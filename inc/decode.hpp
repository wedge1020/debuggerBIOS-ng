#pragma once
////////////////////////////////////////////////////////////////////////////////////////
//
// decode.hpp - machine code to assembly text
//
////////////////////////////////////////////////////////////////////////////////////////

// build the assembly text of an instruction (mnemonic padded to 6 characters,
// then the operands) into `line`
void  decode_to_text  (Text *line, int instruction, int immediate);

// decode an instruction and draw it at (x, y)
void  decode          (int x, int y, int instruction, int immediate);
