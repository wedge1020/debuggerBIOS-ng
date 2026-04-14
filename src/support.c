void  draw_message_screen (int *title, int *message)
{
    int previous_texture  = get_selected_texture ();
    int previous_region   = get_selected_region ();

    clear_screen (error_colors_background);
    set_multiply_color (color_white);
    select_texture (-1);

    // write title
    set_multiply_color (error_colors_title);
    print_at (49, 37, title);

    // draw horizontal line
    select_region (region_white_square);
    set_drawing_scale (640/2, 1);
    draw_region_zoomed_at (0, 60);
    set_drawing_scale (1, 1);

    // write message
    set_multiply_color (error_colors_message);
    print_at (49, 95, message);

    select_texture (previous_texture);
    select_region (previous_region);
}

void  hexit  (int x, int y, int value)
{
    int [9] data;
    int     mask          = 0xF0000000;
    int     shift         = 28;
    int     index         = 0;
    int     byte          = 0;
    int     adjust        = 10;
    float   sx            = 0.0;
    float   sy            = 0.0;

    int     prev_vtex     = get_selected_texture ();
    int     prev_region   = get_selected_region  ();

    select_texture (-1);
    get_drawing_scale (&sx, &sy);
    adjust         = (float) adjust * sx;
    if (sx        != 1.0)
    {
        adjust     = adjust + 1;
    }

    strcpy (data, "0x");
    print_zoomed_at (x, y, data, sx);
    x = x + (adjust * 2);

    for (index = 0; index < 8; index++)
    {
        byte       = (value & mask)    >> shift;
        if (byte  >  9)
            byte   = byte + 0x7;
        byte       = byte + 0x30;

        select_region  (byte);
        draw_region_at (x, y);

        x          = x + adjust;
        shift      = shift - 4;
        mask       = mask >> 4;
    }
    select_texture (prev_vtex);
    select_region  (prev_region);
}

void  hexit_zoomed (int x, int y, int value, float factor)
{
    set_drawing_scale (factor, (factor * 2));
    hexit (x, y, value);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// portit(): display 3 digit IOPort hex value
//
void portit  (int x, int y, int value)
{
    int [4] data;
    int     mask   = 0xF00;
    int     shift  = 8;
    int     index  = 0;
    int     byte   = 0;

    int previous_texture  = get_selected_texture ();
    int previous_region   = get_selected_region ();

    select_texture (-1);

    strcpy (data, "0x");
    print_at (x, y, data);
    x = x + 20;

    for (index = 0; index < 3; index++)
    {
        byte       = (value & mask)    >> shift;
        if (byte  >  9)
            byte   = byte + 0x7;
        byte       = byte + 0x30;

        select_region (byte);
        draw_region_at (x, y);

        x          = x + 10;
        shift      = shift - 4;
        mask       = mask >> 4;
    }
    select_texture (previous_texture);
    select_region  (previous_region);
}

void  print_hex_value (int  x, int  y, int *name, int  value)
{
    // convert the number to hex, always
    // showing the full 8 hex digits
    int [16+1] hex_characters  = "0123456789ABCDEF";
    int [8+1 ] hex_string;

    for (int Digit            = 7;
         Digit               >= 0;
         Digit--)
    {
        hex_string[ Digit ]   = hex_characters[value & 15];
        value               >>= 4;
    }

    hex_string[8]             = 0;

    // join all text parts
    int [60]   text;
    strcpy (text, name);
    strcat (text, " = 0x");
    strcat (text, hex_string);

    // print the text
    print_at (x, y, text);
}

bool  cartridge_connected ()
{
    asm
    {
        "in R0, CAR_Connected"
    }
}

void  request_cartridge ()
{
    int  previous_region   = get_selected_region ();

    draw_message_screen
    (
        "NO CARTRIDGE FOUND",
        "To play a game, please power off\n"
        "your console and insert a game\n"
        "cartridge compatible with Vircon32."
    );

    // draw console diagram
    set_multiply_color (color_white);
    select_region (region_console);
    draw_region_at (400, 207);

    select_region (region_cartridge );
    draw_region_at (469, 76 );

    select_region (region_large_arrow );
    draw_region_at (497, 149 );

    // ensure everything gets drawn
    end_frame ();

    select_region (previous_region);
}

void init_regions (void)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // define all texture regions in bios texture
    //
    select_texture (-1);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // all the characters of the text font
    //
    define_region_matrix (first_region_font, 1, 22, 10, 41, 1, 22, 32, 8, 0);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // white pixel
    //
    select_region (region_white_pixel);
    define_region_topleft (469, 29, 469, 29);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // logo letters
    //
    select_region (region_logo_letters);
    define_region (1, 183, 278, 238, 140, 238);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // logo V
    //
    select_region (region_logo_v);
    define_region_topleft (1, 183, 54,  238);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // logo 32
    //
    select_region (region_logo_32);
    define_region_topleft (190, 183, 278, 238);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // divider_v
    //
    select_region (region_divider_v);
    define_region_topleft (0, 0, 0, 50);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // divider_h
    //
    select_region (region_divider_h);
    define_region_topleft (0, 0, 80, 0);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // logo line
    //
    select_region (region_logo_line);
    define_region (1, 240, 278, 247, 140, 232);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // subtitle
    //
    select_region (region_subtitle);
    define_region (1, 1, 440, 20, 221, 20 );

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // console
    //
    select_region (region_console);
    define_region_topleft (280, 191, 478, 326);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // cartridge
    //
    select_region (region_cartridge);
    define_region_topleft (322, 116, 403, 189);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // large arrow
    //
    select_region (region_large_arrow);
    define_region_topleft (444, 26, 467, 85);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // white square
    //
    select_region (region_white_square);
    define_region_topleft (469, 26, 470, 27);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // clear the screen
    //
    clear_screen (color_black);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // CHECK FOR CARTRIDGE
    //
    if (!cartridge_connected ())
    {
        request_cartridge ();
        asm { "hlt" }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // sanitize screen: ensure that any video parameters that might have
    // been used are restored to expected defaults
    //
    set_multiply_color (color_white);
    set_drawing_point (0, 0);
    select_region (0);

    select_gamepad (DEBUG_GAMEPAD);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// print_zoomed_at(): print "text" at given coordinates, X scale influenced
// by provided factor (1.0 for normal size)
//
void print_zoomed_at (int  drawing_x, int  drawing_y, int *text, float  factor)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // preserve previous texture selection
    //
    int  previous_texture   = get_selected_texture ();
    int  previous_region    = get_selected_region  ();

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // select the BIOS texture
    //
    select_texture (-1);
    
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // begin drawing characters at the given position
    //
    int  initial_drawing_x  = drawing_x;
    
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // until we encounter the null terminator at the end of the string...
    //
    while (*text)
    {
        ////////////////////////////////////////////////////////////////////////////////
        //
        // print this character (character value is region id)
        //
        select_region         (*text);
        set_drawing_scale     (factor,    1.0);
        draw_region_zoomed_at (drawing_x, drawing_y);
        
        ////////////////////////////////////////////////////////////////////////////////
        //
        // advance in x, influenced by any scaling factor
        //
        if (factor         == 1.0)
        {
            drawing_x      += bios_character_width;
        }
        else
        {
            drawing_x      += (bios_character_width * factor) + 1;
        }
        
        ////////////////////////////////////////////////////////////////////////////////
        //
        // execute line breaks
        //
        if (*text          == '\n')
        {
            ////////////////////////////////////////////////////////////////////////////
            //
            // y advances, x is reset
            //
            drawing_x       = initial_drawing_x;
            drawing_y      += bios_character_height;
        }
        
        ////////////////////////////////////////////////////////////////////////////////
        //
        // advance to next character
        //
        ++text;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // restore previous texture and region selection
    //
    select_texture (previous_texture);
    select_region  (previous_region);
}
        
void draw_logo (int  modeflag, int *coffset)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // preserve previous texture selection
    //
    int  previous_texture   = get_selected_texture ();
    int  previous_region    = get_selected_region  ();

    set_blending_mode (blending_alpha);
    set_multiply_color (0xFFFFFFFF);
    clear_screen (color_black);
    select_texture (-1);
    select_region (region_divider_v);
    draw_region_at (0, 0);
    draw_region_at (80, 0);
    select_region (region_divider_h);
    draw_region_at (0, 0);
    draw_region_at (0, 50);
    select_region (region_logo_v);
    set_drawing_scale (0.5, 0.5);
    draw_region_zoomed_at (0, 0);
    select_region (region_logo_32);
    draw_region_zoomed_at (20,0);
    print_zoomed_at (0, 30, "d e b u g g e r", 0.50);
    set_drawing_scale (0.75, 1.5);
    set_drawing_angle (4.712389);
    select_region (66);
    draw_region_rotozoomed_at (64, 30);
    select_region (73);
    draw_region_rotozoomed_at (64, 24);
    select_region (79);
    draw_region_rotozoomed_at (64, 16);
    select_region (83);
    draw_region_rotozoomed_at (64, 8);
    set_multiply_color (color_yellow);
    print_zoomed_at (100, 0,  "DOWN single steps, START to continue, X to escape", 0.50);
    print_zoomed_at (100, 16, "UP for content view (REGS, MEM, STACK, IOPORTS, none)", 0.50);

    switch (modeflag)
    {
        case MODE_REG:
            print_zoomed_at (100, 32, "LEFT/RIGHT cycle formats (hex, int, float)", 0.50);
            break;

        case MODE_RNG:
            print_zoomed_at (100, 32, "LEFT/RIGHT adjusts RNG value by 1, L/R by 100", 0.50);
            break;

        case MODE_STA:
            print_zoomed_at (100, 32, "LEFT/RIGHT adjust BP/SP view", 0.50);
            break;

        case MODE_BTR:
            print_zoomed_at (100, 32, "LEFT/RIGHT adjust backtrace view by 16", 0.50);
            break;

        case MODE_RAM:
        case MODE_MEM:
            print_zoomed_at (100, 32, "LEFT/RIGHT adjust by 16, L/R adjust by 256", 0.50);
            break;
    }

    if (coffset != NULL)
    {
        print_zoomed_at (393, 0, ", Y for mode", 0.50);
    }

    set_multiply_color (color_white);
    
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // restore previous texture and region selection
    //
    select_texture (previous_texture);
    select_region  (previous_region);
}

void zprint_zoomed_at (int  drawing_x, int  drawing_y, int *text, float  factor)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // preserve previous texture and region selection
    //
    int  previous_texture   = get_selected_texture ();
    int  previous_region    = get_selected_region  ();

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // select the BIOS texture
    //
    select_texture (-1);
    
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // declare and initialize variables
    //
    int     pos             = 3;
    int     mask            = 0x000000FF;
    int     shift           = 0;
    int     symbol          = -1;
    int [4] word;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // keep going until we encounter a byte with 0x00 in it (end of string)
    //
    while (symbol          != 0)
    {
        asm { "MOV R10,          {symbol}"
              "MOV [0x003FFBAA], R10" }
        mask                = 0x000000FF;
        shift               = 0;
        for (pos            = 3;
             pos           >= 0;
             pos            = pos - 1)
        {
            ////////////////////////////////////////////////////////////////////////////
            //
            // store into word array
            //
            word[pos]       = ((*text) & mask) >> shift;
            word[pos]       = word[pos] & 0x000000FF;

            ////////////////////////////////////////////////////////////////////////////
            //
            // adjust the bit mask and shift
            //
            mask            = mask << 8;
            shift           = shift + 8;
        }

        for (pos            = 0;
             pos           <  4;
             pos            = pos + 1)
        {
            symbol          = word[pos];
            if (symbol     == 0)
            {
                break;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // symbol value is the region, select it and draw it
            //
            select_region         (symbol);
            set_drawing_scale     (factor,    1.0);
            draw_region_zoomed_at (drawing_x, drawing_y);
            
            ////////////////////////////////////////////////////////////////////////////
            //
            // advance in x, influenced by any scaling factor
            //
            if (factor     == 1.0)
            {
                drawing_x  += bios_character_width;
            }
            else
            {
                drawing_x  += (bios_character_width * factor) + 1;
            }
        }

        text                = text + 1;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // restore previous texture and region selection
    //
    select_texture (previous_texture);
    select_region  (previous_region);
}
