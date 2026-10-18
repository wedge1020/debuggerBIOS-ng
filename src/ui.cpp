////////////////////////////////////////////////////////////////////////////////////////
//
// ui.cpp - drawing: text, numbers, the logo, the message screens
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
// ui_begin(): select the BIOS texture and put the drawing state in order
//
void ui_begin (void)
{
    select_texture     (-1);
    set_blending_mode  (blending_alpha);
    set_multiply_color (color_white);
    set_drawing_scale  (1.0, 1.0);
    set_drawing_angle  (0.0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// ui_text(): draw a string, one font character (= one region) at a time.
//
// The advance of a narrowed character follows the original debugger: 10 pixels
// scaled and truncated, plus one. Line feed starts a new line instead of being
// drawn; so do tab and carriage return, which the current BIOS font draws as
// pictures (a line feed glyph would show up as a stray symbol).
//
void ui_text (int x, int y, int *text, float sx, float sy)
{
    int  home       = x;
    int  advance    = 10;
    int  character  = 0;

    if (sx         != 1.0)
    {
        advance     = (int) (10.0 * sx) + 1;
    }

    set_drawing_scale (sx, sy);

    while (*text)
    {
        character   = *text;

        if (character == 10)
        {
            x       = home;
            y       = y + 20;
        }

        else if ((character != 9) && (character != 13))
        {
            select_region         (character);
            draw_region_zoomed_at (x, y);
            x       = x + advance;
        }

        text        = text + 1;
    }

    set_drawing_scale (1.0, 1.0);
}

void ui_print (int x, int y, int *text)
{
    ui_text (x, y, text, 1.0, 1.0);
}

void ui_small (int x, int y, int *text, float factor)
{
    ui_text (x, y, text, factor, 1.0);
}

// ui_hex(): "0x" and 8 hex digits. As in the original, only the "0x" is scaled
// (across, by `factor`); the digits are drawn at their normal size, with their
// spacing tightened by `factor`, which makes them overlap a little.
void ui_hex (int x, int y, int value, float factor)
{
    int   shift   = 28;
    int   digit   = 0;
    int   adjust  = 0;
    int   index   = 0;

    adjust        = (int) (10.0 * factor);
    if (factor   != 1.0)
    {
        adjust    = adjust + 1;
    }

    ui_text            (x, y, "0x", factor, 1.0);
    x             = x + (adjust * 2);

    select_texture     (-1);
    set_drawing_scale  (1.0, 1.0);
    for (index    = 0;
         index   <  8;
         index    = index + 1)
    {
        digit     = (value >> shift) & 0xF;
        if (digit >  9)
        {
            digit = digit + 7;
        }
        select_region  (digit + 0x30);
        draw_region_at (x, y);

        x         = x + adjust;
        shift     = shift - 4;
    }
}

void ui_port (int x, int y, int value)
{
    int   buffer[6];
    Text  line;

    line.init (buffer);
    line.str  ("0x");
    line.hex  (value, 3);
    ui_text   (x, y, buffer, 1.0, 1.0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// ui_view_title(): the heading every resource view starts with (at the right of
// the screen), followed by a rule across the view's width
//
void ui_view_title (int *title)
{
    ui_print              (464, 0, title);
    select_region         (region_divider_h);
    draw_region_at        (464, 20);
    draw_region_at        (560, 20);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// draw_logo(): clear the screen and draw the debuggerBIOS logo in the top left,
// with the controls help along the top
//
void draw_logo (int mode, int keyboard)
{
    set_blending_mode     (blending_alpha);
    set_multiply_color    (color_white);
    clear_screen          (color_black);
    select_texture        (-1);

    select_region         (region_divider_v);
    draw_region_at        (0, 0);
    draw_region_at        (80, 0);
    select_region         (region_divider_h);
    draw_region_at        (0, 0);
    draw_region_at        (0, 50);

    select_region         (region_logo_v);
    set_drawing_scale     (0.5, 0.5);
    draw_region_zoomed_at (0, 0);
    select_region         (region_logo_32);
    draw_region_zoomed_at (20, 0);
    ui_small              (0, 30, "d e b u g g e r", 0.5);

    // "BIOS", turned on its side
    set_drawing_scale     (0.75, 1.5);
    set_drawing_angle     (4.712389);
    select_region         ('B');
    draw_region_rotozoomed_at (64, 30);
    select_region         ('I');
    draw_region_rotozoomed_at (64, 24);
    select_region         ('O');
    draw_region_rotozoomed_at (64, 16);
    select_region         ('S');
    draw_region_rotozoomed_at (64, 8);
    set_drawing_angle     (0.0);
    set_drawing_scale     (1.0, 1.0);

    set_multiply_color    (color_yellow);
    ui_small              (100, 0,
                           "DOWN steps, START continues, X escapes (on release)", 0.5);
    ui_small              (100, 16,
                           "UP for content view (REGS, MEM, STACK, IOPORTS, none)", 0.5);

    switch (mode)
    {
        case MODE_REG:
            ui_small (100, 32, "LEFT/RIGHT cycle formats (hex, int, float)", 0.5);
            break;

        case MODE_RNG:
            ui_small (100, 32, "LEFT/RIGHT adjusts RNG value by 1, L/R by 100", 0.5);
            break;

        case MODE_STA:
            ui_small (100, 32, "LEFT/RIGHT adjust BP/SP view", 0.5);
            break;

        case MODE_BTR:
            ui_small (100, 32, "LEFT/RIGHT adjust backtrace view by 16", 0.5);
            break;

        case MODE_RAM:
        case MODE_MEM:
            ui_small (100, 32, "LEFT/RIGHT adjust by 16, L/R adjust by 256", 0.5);
            break;

        case MODE_INP:
            ui_small (100, 32, "LEFT/RIGHT choose the gamepad to show", 0.5);
            break;

        default:
            if (keyboard)
            {
                ui_small (100, 32, "keyboard: arrows, Enter=START, Esc=X, [ ]=L R", 0.5);
            }
            break;
    }

    set_multiply_color    (color_white);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// draw_message_screen(): a full-screen title and message on the error colors
//
void draw_message_screen (int *title, int *message)
{
    set_blending_mode     (blending_alpha);
    set_drawing_angle     (0.0);
    set_drawing_scale     (1.0, 1.0);
    clear_screen          (error_colors_background);
    select_texture        (-1);

    set_multiply_color    (error_colors_title);
    ui_print              (49, 37, title);

    select_region         (region_white_square);
    set_multiply_color    (error_colors_message);
    set_drawing_scale     (640.0 / 2.0, 1.0);
    draw_region_zoomed_at (0, 60);
    set_drawing_scale     (1.0, 1.0);

    ui_print              (49, 95, message);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// request_cartridge(): the "no cartridge" screen
//
void request_cartridge (void)
{
    draw_message_screen
    (
        "NO CARTRIDGE FOUND",
        "To play a game, please power off\n"
        "your console and insert a game\n"
        "cartridge compatible with Vircon32."
    );

    // draw console diagram
    set_multiply_color (color_white);
    select_region      (region_console);
    draw_region_at     (400, 207);

    select_region      (region_cartridge);
    draw_region_at     (469, 76);

    select_region      (region_large_arrow);
    draw_region_at     (497, 149);

    // ensure everything gets drawn
    end_frame ();
}

////////////////////////////////////////////////////////////////////////////////////////
//
// init_regions(): define the regions of the BIOS texture, and check that a
// cartridge is there to debug. The regions match the current StandardBios
// texture (see docs/TEXTURE.md).
//
void init_regions (void)
{
    select_texture (-1);

    // all the characters of the text font: a 32x8 grid of 10x20 cells
    define_region_matrix (first_region_font, 1, 22, 10, 41, 1, 22, 32, 8, 0);

    select_region          (region_white_pixel);
    define_region_topleft  (469, 29, 469, 29);

    select_region          (region_logo_letters);
    define_region          (1, 183, 278, 238, 140, 238);

    select_region          (region_logo_v);
    define_region_topleft  (1, 183, 54, 238);

    select_region          (region_logo_32);
    define_region_topleft  (190, 183, 278, 238);

    select_region          (region_divider_v);
    define_region_topleft  (0, 0, 0, 50);

    select_region          (region_divider_h);
    define_region_topleft  (0, 0, 80, 0);

    select_region          (region_logo_line);
    define_region          (1, 240, 278, 247, 140, 232);

    select_region          (region_subtitle);
    define_region          (1, 1, 440, 20, 221, 20);

    select_region          (region_console);
    define_region_topleft  (280, 191, 478, 326);

    select_region          (region_cartridge);
    define_region_topleft  (322, 116, 403, 189);

    select_region          (region_large_arrow);
    define_region_topleft  (444, 26, 467, 85);

    select_region          (region_white_square);
    define_region_topleft  (469, 26, 470, 27);

    clear_screen (color_black);

    // no cartridge: nothing to debug
    if (read_port (PORT_CAR_CONNECTED) == 0)
    {
        request_cartridge ();
        asm { "hlt" }
    }

    // leave the video state the way a CART expects to find it
    set_blending_mode  (blending_alpha);
    set_multiply_color (color_white);
    set_drawing_point  (0, 0);
    set_drawing_scale  (1.0, 1.0);
    set_drawing_angle  (0.0);
    select_region      (0);
    select_gamepad     (0);
}
