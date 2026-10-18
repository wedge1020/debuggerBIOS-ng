////////////////////////////////////////////////////////////////////////////////////////
//
// views.cpp - the resource views, drawn at the right of the screen
//
// Each view is a title and a column of rows: a label at x=464 and a value at
// x=560. Rows are 20 pixels apart, the first below the rule under the title.
// The rows are written one line each, with the helpers below, in place of the
// original's block of assembly per value; ports are read with read_port().
//
////////////////////////////////////////////////////////////////////////////////////////

#define  VIEW_LABEL_X            464
#define  VIEW_VALUE_X            560
#define  VIEW_FIRST_Y            20

////////////////////////////////////////////////////////////////////////////////////////
//
// view row helpers: `row` counts from 0
//
static void row_decimal (int row, int *label, int value)
{
    int   buffer[14];
    Text  line;

    ui_print     (VIEW_LABEL_X, VIEW_FIRST_Y + (row * 20), label);
    line.init    (buffer);
    line.dec     (value);
    ui_print     (VIEW_VALUE_X, VIEW_FIRST_Y + (row * 20), buffer);
}

static void row_hex (int row, int *label, int value)
{
    ui_print     (VIEW_LABEL_X, VIEW_FIRST_Y + (row * 20), label);
    ui_hex       (VIEW_VALUE_X, VIEW_FIRST_Y + (row * 20), value, 0.75);
}

static void row_float (int row, int *label, int bits)
{
    int   buffer[40];
    Text  line;

    ui_print     (VIEW_LABEL_X, VIEW_FIRST_Y + (row * 20), label);
    line.init    (buffer);
    line.flt     (bits);
    ui_print     (VIEW_VALUE_X, VIEW_FIRST_Y + (row * 20), buffer);
}

static void row_text (int row, int *label, int *text)
{
    ui_print     (VIEW_LABEL_X, VIEW_FIRST_Y + (row * 20), label);
    ui_print     (VIEW_VALUE_X, VIEW_FIRST_Y + (row * 20), text);
}

static void row_bool (int row, int *label, int value)
{
    if (value == 0)
    {
        row_text (row, label, "false");
    }
    else
    {
        row_text (row, label, "true");
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// draw_view(): draw the selected view (nothing for MODE_NONE)
//
void Debugger::draw_view (void)
{
    if (mode == MODE_NONE)
    {
        return;
    }

    ui_begin ();

    switch (mode)
    {
        case MODE_REG:  view_registers ();  break;
        case MODE_RAM:  view_ram ();        break;
        case MODE_STA:  view_stack ();      break;
        case MODE_BTR:  view_backtrace ();  break;
        case MODE_TIM:  view_timer ();      break;
        case MODE_RNG:  view_rng ();        break;
        case MODE_GPU:  view_gpu ();        break;
        case MODE_SPU:  view_spu ();        break;
        case MODE_INP:  view_input ();      break;
        case MODE_CAR:  view_cartridge ();  break;
        case MODE_MEM:  view_memcard ();    break;
    }

    set_multiply_color (color_white);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// registers: the CART's 16 registers in hex, integer or float, then the
// instruction about to run
//
void Debugger::view_registers (void)
{
    int  *regs     = (int *) ADDR_CART_REGISTERS;
    int   buffer[40];
    Text  line;
    int   index    = 0;
    int   y        = 0;
    int   value    = 0;

    ui_print              (490, 0, "register view");
    select_region         (region_divider_h);
    set_drawing_scale     (150.0 / 81.0, 1.0);
    draw_region_zoomed_at (490, 20);
    set_drawing_scale     (1.0, 1.0);

    for (index    = 0;
         index   <  16;
         index    = index + 1)
    {
        y         = 18 + (index * 18);

        line.init (buffer);
        line.reg  (index);
        line.add  (':');
        ui_print  (490, y, buffer);

        value     = regs[index];
        switch (view[MODE_REG])
        {
            case FORMAT_HEX:
                ui_hex (540, y, value, 0.75);
                break;

            case FORMAT_INTEGER:
                line.init (buffer);
                line.dec  (value);
                ui_print  (540, y, buffer);
                break;

            case FORMAT_FLOAT:
                line.init (buffer);
                line.flt  (value);
                ui_print  (540, y, buffer);
                break;
        }
    }

    y             = 18 + (16 * 18);
    ui_print (490, y, "IP:");
    ui_hex   (540, y, (int) pc, 0.75);

    value         = *pc;
    ui_print (490, y + 18, "IR:");
    ui_hex   (540, y + 18, value, 0.75);

    if (((value >> 25) & 1) == 1)
    {
        ui_print (490, y + 36, "IV:");
        ui_hex   (540, y + 36, *(pc + 1), 0.75);
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// RAM: 16 consecutive words, from the address chosen with LEFT/RIGHT, L/R
//
void Debugger::view_ram (void)
{
    int  index     = 0;
    int  address   = 0;

    ui_view_title ("memaddr   value");

    for (index     = 0;
         index    <  16;
         index     = index + 1)
    {
        address    = view[MODE_RAM] + index;
        ui_hex   (464, 20 + (index * 20), address, 0.75);
        ui_small (544, 20 + (index * 20), ":", 0.75);
        ui_hex   (560, 20 + (index * 20), *((int *) address), 0.75);
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// stack: the words from the CART's SP up to its BP ("how many?" is chosen with
// LEFT/RIGHT), then BP and what is above it. All the numbers are addresses on
// the CART's stack, which ends at ADDR_CART_STACK_BASE.
//
void Debugger::view_stack (void)
{
    int  *regs     = (int *) ADDR_CART_REGISTERS;
    int   sp       = regs[15];
    int   bp       = regs[14];
    int   limit    = view[MODE_STA];
    int   buffer[20];
    Text  line;
    int   index    = 0;
    int   pos      = 20;

    ui_view_title ("stack     value");

    ui_print (464, 20, "SP:");
    ui_hex   (560, 20, sp, 0.75);

    for (index     = 0;
         index    <= limit;
         index     = index + 1)
    {
        if (((sp + index) <  0) ||
            ((sp + index) >  ADDR_CART_STACK_BASE) ||
            (bp           <  (sp + index)))
        {
            break;
        }

        pos        = pos + 16;
        line.init  (buffer);
        line.str   ("[SP+");
        line.dec   (index);
        line.str   ("]:");
        ui_print   (464, pos, buffer);
        ui_hex     (560, pos, *((int *) (sp + index)), 0.75);

        if (bp    == (sp + index))
        {
            break;
        }
    }

    pos            = pos + 16;
    ui_print (464, pos, "BP:");
    ui_hex   (560, pos, bp, 0.75);

    for (index     = 0;
         index    <  (18 - limit);
         index     = index + 1)
    {
        if (((bp + index) <  0) ||
            ((bp + index) >  ADDR_CART_STACK_BASE))
        {
            break;
        }

        pos        = pos + 16;
        line.init  (buffer);
        line.str   ("[BP+");
        line.dec   (index);
        line.str   ("]:");
        ui_print   (464, pos, buffer);
        ui_hex     (560, pos, *((int *) (bp + index)), 0.75);
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// backtrace: the entry address of each subroutine the CART is in, the one
// called last at the top. LEFT/RIGHT page through them, 16 at a time.
//
void Debugger::view_backtrace (void)
{
    int   buffer[12];
    Text  line;
    int   index    = 0;
    int   pos      = 20;

    ui_view_title ("backtrace");

    for (index     = view[MODE_BTR] + 15;
         index    >= view[MODE_BTR];
         index     = index - 1)
    {
        if (index >= trace.count ())
        {
            continue;
        }

        line.init (buffer);
        line.add  ('[');
        line.dec  (index);
        line.str  ("]:");
        ui_small  (464, pos, buffer, 0.75);
        ui_hex    (524, pos, trace.get (index), 0.75);
        pos        = pos + 20;
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// timer: the date and time of the console, and the frame and cycle counts the
// CART sees (not the console's own: see emulate())
//
void Debugger::view_timer (void)
{
    ui_view_title ("TIM port value");

    row_hex     (0, "Date:",     read_port (PORT_TIM_DATE));
    row_hex     (1, "Time:",     read_port (PORT_TIM_TIME));
    row_decimal (2, "Frame #:",  *((int *) ADDR_PORT_TIMFRAME));
    row_decimal (3, "Cycles:",   *((int *) ADDR_PORT_TIMCYCLE));
    row_decimal (4, "Console:",  read_port (PORT_TIM_FRAMECOUNTER));
}

////////////////////////////////////////////////////////////////////////////////////////
//
// RNG: the value the CART will read next (LEFT/RIGHT, L/R change it)
//
void Debugger::view_rng (void)
{
    ui_view_title ("RNG port value");

    row_decimal (0, "RAND:", *((int *) ADDR_PORT_RNGVALUE));
}

////////////////////////////////////////////////////////////////////////////////////////
//
// GPU: the CART's GPU state, kept in the shadow words while the debugger has the
// hardware, and the geometry of the region it has selected
//
void Debugger::view_gpu (void)
{
    int  *shadow   = (int *) ADDR_SHADOW;
    int   blending = shadow[SHADOW_BLEND];

    ui_view_title ("GPU port value");

    row_hex     (0, "Clear:",    shadow[SHADOW_CLEAR]);
    row_hex     (1, "Multiply:", shadow[SHADOW_MULTIPLY]);

    if (blending == blending_alpha)
    {
        row_text (2, "Blending:", "alpha");
    }
    else if (blending == blending_add)
    {
        row_text (2, "Blending:", "add");
    }
    else
    {
        row_text (2, "Blending:", "subtract");
    }

    row_decimal (3, "Texture:",  shadow[SHADOW_TEXTURE]);
    row_decimal (4, "Region:",   shadow[SHADOW_REGION]);
    row_decimal (5, "DrawX:",    shadow[SHADOW_DRAWX]);
    row_decimal (6, "DrawY:",    shadow[SHADOW_DRAWY]);
    row_float   (7, "ScaleX:",   shadow[SHADOW_SCALEX]);
    row_float   (8, "ScaleY:",   shadow[SHADOW_SCALEY]);
    row_float   (9, "Angle:",    shadow[SHADOW_ANGLE]);

    // the region's geometry can only be asked of the GPU for the region that is
    // selected, so select the CART's, ask, and go back to the BIOS texture
    select_texture (shadow[SHADOW_TEXTURE]);
    select_region  (shadow[SHADOW_REGION]);

    row_decimal (10, "MinX:",     read_port (PORT_GPU_REGION_MINX));
    row_decimal (11, "MinY:",     read_port (PORT_GPU_REGION_MINY));
    row_decimal (12, "MaxX:",     read_port (PORT_GPU_REGION_MAXX));
    row_decimal (13, "MaxY:",     read_port (PORT_GPU_REGION_MAXY));
    row_decimal (14, "HotspotX:", read_port (PORT_GPU_REGION_HOTX));
    row_decimal (15, "HotspotY:", read_port (PORT_GPU_REGION_HOTY));

    select_texture (-1);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// SPU: the sound unit, as the CART left it
//
void Debugger::view_spu (void)
{
    int  state     = 0;

    ui_view_title ("SPU port value");

    row_float   (0,  "Volume:",   read_port (PORT_SPU_VOLUME));
    row_decimal (1,  "Sound #:",  read_port (PORT_SPU_SOUND));
    row_decimal (2,  "Channel#:", read_port (PORT_SPU_CHANNEL));
    row_hex     (3,  "SoundLen:", read_port (PORT_SPU_SOUND_LENGTH));
    row_bool    (4,  "PlayLoop:", read_port (PORT_SPU_SOUND_LOOP));
    row_hex     (5,  "LoopStrt:", read_port (PORT_SPU_LOOP_START));
    row_hex     (6,  "LoopEnd:",  read_port (PORT_SPU_LOOP_END));

    state          = read_port (PORT_SPU_CHAN_STATE);
    if (state     == 0x40)
    {
        row_text (7, "ChanStat:", "stopped");
    }
    else if (state == 0x41)
    {
        row_text (7, "ChanStat:", "paused");
    }
    else if (state == 0x42)
    {
        row_text (7, "ChanStat:", "playing");
    }
    else
    {
        row_text (7, "ChanStat:", "?");
    }

    row_decimal (8,  "ChnSet:",   read_port (PORT_SPU_CHAN_SOUND));
    row_float   (9,  "ChanVol:",  read_port (PORT_SPU_CHAN_VOLUME));
    row_float   (10, "ChanSpd:",  read_port (PORT_SPU_CHAN_SPEED));
    row_bool    (11, "ChanLoop:", read_port (PORT_SPU_CHAN_LOOP));
    row_decimal (12, "ChanPos:",  read_port (PORT_SPU_CHAN_POSITION));
}

////////////////////////////////////////////////////////////////////////////////////////
//
// INP: the controls of a gamepad (LEFT/RIGHT choose which). The value of a control
// is the number of frames it has been held, or minus the frames it has been up.
//
void Debugger::view_input (void)
{
    ui_view_title ("INP port value");

    select_gamepad (view[MODE_INP]);

    row_decimal (0,  "Gamepad:",   view[MODE_INP]);
    row_bool    (1,  "Connect:",   read_port (PORT_INP_CONNECTED));
    row_decimal (2,  "Left:",      read_port (PORT_INP_LEFT));
    row_decimal (3,  "Right:",     read_port (PORT_INP_RIGHT));
    row_decimal (4,  "Up:",        read_port (PORT_INP_UP));
    row_decimal (5,  "Down:",      read_port (PORT_INP_DOWN));
    row_decimal (6,  "Start:",     read_port (PORT_INP_START));
    row_decimal (7,  "A:",         read_port (PORT_INP_A));
    row_decimal (8,  "B:",         read_port (PORT_INP_B));
    row_decimal (9,  "X:",         read_port (PORT_INP_X));
    row_decimal (10, "Y:",         read_port (PORT_INP_Y));
    row_decimal (11, "L:",         read_port (PORT_INP_L));
    row_decimal (12, "R:",         read_port (PORT_INP_R));

    select_gamepad (DEBUG_GAMEPAD);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// CAR: the cartridge
//
void Debugger::view_cartridge (void)
{
    ui_view_title ("CAR port value");

    row_bool    (0, "Connect:", read_port (PORT_CAR_CONNECTED));
    row_hex     (1, "ROMSize:", read_port (PORT_CAR_ROMSIZE));
    row_decimal (2, "#VTEX:",   read_port (PORT_CAR_TEXTURES));
    row_decimal (3, "#VSND:",   read_port (PORT_CAR_SOUNDS));
}

////////////////////////////////////////////////////////////////////////////////////////
//
// MEM: the memory card, if there is one: its name (the text at its very start)
// and 16 words from the address chosen with LEFT/RIGHT, L/R
//
void Debugger::view_memcard (void)
{
    int   name[17];
    int  *card     = (int *) ADDR_MEMCARD;
    int   index    = 0;
    int   address  = 0;

    ui_view_title ("MEM port value");

    if (read_port (PORT_MEM_CONNECTED) == 0)
    {
        row_text (0, "Connect:", "false");
        return;
    }

    // a bounded copy: the card is not obliged to hold a terminated string
    while ((index < 16) &&
           (card[index] != 0))
    {
        name[index]  = card[index];
        index        = index + 1;
    }
    name[index]      = 0;
    ui_print (464, 20, name);

    for (index       = 0;
         index      <  16;
         index       = index + 1)
    {
        address      = view[MODE_MEM] + index;
        ui_hex   (464, 40 + (index * 20), address, 0.75);
        ui_small (544, 40 + (index * 20), ":", 0.75);
        ui_hex   (560, 40 + (index * 20), *((int *) address), 0.75);
    }
}
