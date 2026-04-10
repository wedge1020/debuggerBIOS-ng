//
// Source - https://stackoverflow.com/a/6782480
//
// Posted by ryyst, modified by community. See post 'Timeline' for change history
//
// Retrieved 2026-04-06, License - CC BY-SA 3.0
//
// compile with: gcc -Wall -o encode encode.c -Wno-char-subscripts -Wno-pointer-sign
//
// run for header:  ./encode
// run for debug:   ./encode BASE64DATA
// run with offset: ./encode 0xOFFSET BASE64DATA
//
// Will output results in "binary" (as char) to STDOUT
//
////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef  unsigned char  uc;

uc   *base64_decode        (char *, size_t, size_t *);
void  build_decoding_table ();
void  base64_cleanup       ();

static char   encoding_table[]  = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                    'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                    '4', '5', '6', '7', '8', '9', '+', '/' };

static char *decoding_table     = NULL;
//static int   mod_table[]     = { 0, 2, 1 };

int  main (int  argc, char **argv)
{
    size_t  ilen         = 0;
    size_t  olen         = 0;
    char   *string       = NULL;
    int     addr[4];
    int     index        = 0;
    int     offset       = 0;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // header mode: output just the "V32-TEXT" header
    //
    if (argc            == 1)
    {
        fprintf (stdout, "V32-TEXT");
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // demo mode: do the base64 decoding on the sole argument to verify
    //
    else if (argc       == 2)
    {
        ilen             = strlen (argv[1]);

        fprintf (stdout, "input: '%s'\n", argv[1]);

        string           = base64_decode (argv[1], ilen, &olen);

        fprintf (stdout, "decoded: '%s' (%lu)\n", string, olen);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // data mode: take the offset and the base64 value and output them
    // in "binary"
    //
    else
    {
        offset           = strtol (argv[1], NULL, 16);
        fprintf (stderr, "offset: 0x%.8X\n", offset);
        for (index       = 3;
             index      >= 0;
             index       = index - 1)
        {
            addr[index]  = offset % 256;
            fprintf (stderr, "[%d] %X\n", index, addr[index]);
            offset       = offset / 256;
        }

        for (index       = 0;
             index      <  4;
             index       = index + 1)
        {
            fprintf (stdout, "%c", addr[index]);
        }

        ilen             = strlen (argv[2]);
        string           = base64_decode (argv[2], ilen, &olen);

        fprintf (stdout, "%c%s", (char) olen, string);
    }

    return (0);
}    

/*
char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length) {

    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = malloc(*output_length);
    if (encoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    return encoded_data;
}
*/

uc *base64_decode (char   *data,
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
        sextet_a                 = (data[index] == '=') ? 0 & index++ :
                                   decoding_table[data[index++]];
        sextet_b                 = (data[index] == '=') ? 0 & index++ :
                                   decoding_table[data[index++]];
        sextet_c                 = (data[index] == '=') ? 0 & index++ :
                                   decoding_table[data[index++]];
        sextet_d                 = (data[index] == '=') ? 0 & index++ :
                                   decoding_table[data[index++]];

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
