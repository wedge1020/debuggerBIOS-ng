//
// bincode.c: C program to process offsets and string data for embedding into
// Vircon32 VBIN files as part of debuggerBIOS-ng's C code debugging features
//
// Contains base64 decoding logic originally sourced from:
//
// https://stackoverflow.com/a/6782480
//
// Posted by ryyst, modified by community. See post 'Timeline' for change history
//
// Retrieved 2026-04-06, License - CC BY-SA 3.0
//
// compile with: gcc -Wall -o bincode bincode.c -Wno-char-subscripts -Wno-pointer-sign
//
// run for header:  ./bincode
// run for offset:  ./bincode 0xOFFSET
// run for string:  ./bincode string BASE64DATA
//
// Will output results in "binary" (as char) to STDOUT, with the intent is to
// use I/O redirection to append the data to the desired VBIN file amidst the
// cartridge build process.
//
// This program is called by the `inject-debug.sh` script
//
////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define  MODE_NONE      0
#define  MODE_HEADER    1
#define  MODE_OFFSET    2
#define  MODE_STRING    4
#define  MODE_ENDIAN    8
#define  MODE_VERBOSE  16
#define  MODE_HELP     32

typedef  unsigned char  uc;
typedef  struct option  GetOpt;

uc   *base64_decode        (char *, size_t, size_t *);
void  build_decoding_table ();
void  base64_cleanup       ();
void  process_offset       (int,    int);

static char   encoding_table[]  = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                    'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                    '4', '5', '6', '7', '8', '9', '+', '/' };

static char *decoding_table  = NULL;

int  main (int  argc, char **argv)
{
    size_t  ilen                          = 0;
    size_t  olen                          = 0;
    char   *string                        = NULL;
    char   *string_arg                    = NULL;
    char   *string_data                   = NULL;
    int     index                         = 0;
    int     count                         = 0;
    int     mode                          = MODE_NONE;
    int     offset                        = 0;
    int     opt                           = 0;
    int     option_index                  = 0;

    GetOpt  long_options[]                = {
        { "header",        no_argument,       0, 'H' },
        { "offset",        required_argument, 0, 'o' },
        { "string",        required_argument, 0, 's' },
        { "little-endian", no_argument,       0, 'e' },
        { "big-endian",    no_argument,       0, 'E' },
        { "verbose",       no_argument,       0, 'v' },
        { "help",          no_argument,       0, 'h' },
        { 0,               0,                 0,  0  }
    };

    opt                                   = getopt_long (argc, argv, "Ho:s:eEvh",
                                                         long_options, &option_index);

    while (opt                           != -1)
    {
        switch (opt)
        {
            case 'H':
                mode                      = mode | MODE_HEADER;
                break;

            case 'o':
                mode                      = mode | MODE_OFFSET;
                offset                    = strtol (optarg, NULL, 16);
                break;

            case 's':
                mode                      = mode | MODE_STRING;
                string_arg                = (char *) malloc (sizeof (char) * strlen (optarg));
                strcpy (string_arg, optarg);
                break;

            case 'e':
                mode                      = mode | MODE_ENDIAN;
                break;

            case 'v':
                mode                      = mode | MODE_VERBOSE;
                break;

            case 'h':
                break;
        }
        opt                               = getopt_long (argc, argv, "Ho:s:eEvh",
                                                         long_options, &option_index);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // header mode: display the "V32-TEXT" header
    //
    if (MODE_HEADER                      == (mode & MODE_HEADER))
    {
        if (MODE_VERBOSE                 == (mode & MODE_VERBOSE))
        {
            fprintf (stdout, "[verbose] HEADER:  \"");
        }

        if ((mode & MODE_ENDIAN)         == MODE_ENDIAN)
        {
            fprintf (stdout, "-23VTXET");
        }
        else
        {
            fprintf (stdout, "V32-TEXT");
        }

        if (MODE_VERBOSE                 == (mode & MODE_VERBOSE))
        {
            fprintf (stdout, "\"\n");
        }
    }

    if (MODE_OFFSET                      == (mode & MODE_OFFSET))
    {
        process_offset (mode, offset);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // data mode: ignore the first argument, take the base64 value and output its
    // size then the string in "binary"
    //
    if (MODE_STRING                      == (mode & MODE_STRING))
    {
        ilen                              = strlen (string_arg);
        string                            = base64_decode (string_arg, ilen, &olen);
        string_data                       = (char *) calloc (sizeof (char), 
                                                             (olen + (4 - (olen % 4))));
        strcpy (string_data, string);

        ilen                              = olen;

        olen                              = olen + (4 - (olen % 4));
        olen                              = olen / 4;

        process_offset (mode, olen);

		if (MODE_VERBOSE                 == (mode & MODE_VERBOSE))
		{
			fprintf (stdout, "[verbose] string:  ");
		}

        for (count                        = 0;
             count                       <  olen;
             count                        = count + 1)
        {
            if (MODE_ENDIAN              == (mode & MODE_ENDIAN))
            {
                for (index                = 3;
                     index               >= 0;
                     index                = index - 1)
                {
                    if (MODE_VERBOSE     == (mode & MODE_VERBOSE))
                    {
                        fprintf (stdout, "%.2hhX ", *(string_data+((count * 4) + index)));
                    }
                    else
                    {
                        fprintf (stdout, "%c",      *(string_data+((count * 4) + index)));
                    }
                }
            }
            else
            {
                for (index                = 0;
                     index               <  4;
                     index                = index + 1)
                {
                    if (MODE_VERBOSE     == (mode & MODE_VERBOSE))
                    {
                        fprintf (stdout, "%.2hhX ", *(string_data+((count * 4) + index)));
                    }
                    else
                    {
                        fprintf (stdout, "%c",      *(string_data+((count * 4) + index)));
                    }
                }
            }
        }

        if (MODE_VERBOSE                 == (mode & MODE_VERBOSE))
        {
            fprintf (stdout, "\n");
        }
    }

    return (0);
}    

unsigned char *base64_decode (char   *data,
                              size_t  input_length,
                              size_t *output_length)
{
    uc       *decoded_data       = NULL;
    int       index              = 0;
    int       count              = 0;
    uint32_t  sextet_a           = 0;
    uint32_t  sextet_b           = 0;
    uint32_t  sextet_c           = 0;
    uint32_t  sextet_d           = 0;
    uint32_t  triple             = 0;

    if (decoding_table          == NULL)
    {
        build_decoding_table ();
    }

    if ((input_length % 4)      != 0)
    {
        return (NULL);
    }

    *output_length               = input_length / 4 * 3;
    if (data[input_length-1]    == '=')
    {
        (*output_length)--;
    }

    if (data[input_length-2]    == '=')
    {
        (*output_length)--;
    }

    decoded_data                 = malloc (*output_length);
    if (decoded_data            == NULL)
    {
        return (NULL);
    }

    for (index                   = 0,
         count                   = 0;
         index                  <  input_length;)
    {
        sextet_a                 = (data[index] == '=') ? 0 & index++ : decoding_table[data[index++]];
        sextet_b                 = (data[index] == '=') ? 0 & index++ : decoding_table[data[index++]];
        sextet_c                 = (data[index] == '=') ? 0 & index++ : decoding_table[data[index++]];
        sextet_d                 = (data[index] == '=') ? 0 & index++ : decoding_table[data[index++]];

        triple                   = (sextet_a << 3 * 6) +
                                   (sextet_b << 2 * 6) +
                                   (sextet_c << 1 * 6) +
                                   (sextet_d << 0 * 6);

        if (count               < *output_length)
        {
            decoded_data[count]  = (triple >> 2 * 8) & 0xFF;
            count                = count + 1;
        }

        if (count               < *output_length)
        {
            decoded_data[count]  = (triple >> 1 * 8) & 0xFF;
            count                = count + 1;
        }

        if (count               < *output_length)
        {
            decoded_data[count]  = (triple >> 0 * 8) & 0xFF;
            count                = count + 1;
        }
    }

    return (decoded_data);
}

void  build_decoding_table ()
{
    int  index      = 0;
    decoding_table  = malloc (256);

    for (index      = 0;
         index     <  64;
         index      = index + 1)
    {
        decoding_table[(unsigned char) encoding_table[index]] = index;
    }
}

void  base64_cleanup ()
{
    free (decoding_table);
}

void  process_offset (int  mode, int  offset)
{
    int  index                = 0;
    int  addr[4];

    if (MODE_VERBOSE         == (mode & MODE_VERBOSE))
    {
        fprintf (stdout, "[verbose] OFFSET:  0x%.8X\n", offset);
    }

    for (index                = 3;
         index               >= 0;
         index                = index - 1)
    {
        addr[index]           = offset & 0x000000FF;
        offset                = offset >> 8;
        if (MODE_VERBOSE     == (mode & MODE_VERBOSE))
        {
            fprintf (stdout, "[verbose] addr[%d]: %.2hhX\n", index, addr[index]);
        }
    }

    if (MODE_VERBOSE         == (mode & MODE_VERBOSE))
    {
        fprintf (stdout, "[verbose] offset:  ");
    }

    if ((mode & MODE_ENDIAN) == MODE_ENDIAN)
    {
        for (index            = 3;
             index           >= 0;
             index            = index - 1)
        {
            if (MODE_VERBOSE == (mode & MODE_VERBOSE))
            {
                fprintf (stdout, "%.2hhX ", addr[index]);
            }
            else
            {
                fprintf (stdout, "%c",      addr[index]);
            }
        }
    }
    else
    {
        for (index            = 0;
             index           <  4;
             index            = index + 1)
        {
            if (MODE_VERBOSE == (mode & MODE_VERBOSE))
            {
                fprintf (stdout, "%.2hhX ", addr[index]);
            }
            else
            {
                fprintf (stdout, "%c",    addr[index]);
            }
        }
    }

    if (MODE_VERBOSE         == (mode & MODE_VERBOSE))
    {
        fprintf (stdout, "\n");
    }
}
