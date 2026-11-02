#include <stdio.h>
#include <string.h>
#include "common.h"
#include "encode.h"
#include "decode.h"

/* Text colors */
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define RESET   "\033[0m"

int main(int argc, char *argv[])
{
    // Create structure variable
    EncodeInfo encInfo;

    // Check whether arguments are given
    if (argc == 1)
    {
        printf(RED "ERROR: Insufficient arguments\n" RESET);
        printf(RED "Usage: ./a.out -e beautiful.bmp secret.txt [output.bmp]\n" RESET);
        return failure;
    }

    // Check the operation
    int opr = check_operation(argv[1]);

    if (opr == encode)
    {
        // Check encoding arguments
        if (argc < 4 || argc > 5)
        {
            printf(RED "ERROR: Invalid number of encoding arguments\n" RESET);
            return failure;
        }

        // Validate encoding arguments
        if (validate_encode_args(argv, &encInfo) == failure)
        {
            printf(RED "ERROR: Invalid encoding arguments\n" RESET);
            return failure;
        }

        // Start encoding
        if (do_encoding(argv, &encInfo) == failure)
        {
            printf(RED "ERROR: Encoding failed\n" RESET);
            return failure;
        }
    }
    
    else if (opr == decode)
    {
       DecodeInfo decInfo;

        if (argc < 3 || argc > 4)
        {
            printf("ERROR: Invalid number of decoding arguments\n");
            return failure;
        }

        if (validate_decode_args(argc, argv, &decInfo) == failure)
        {
            printf("ERROR: Invalid decoding arguments\n");
            return failure;
        }

        if (do_decoding(&decInfo) == failure)
        {
            printf("ERROR: Decoding failed\n");
            return failure;
        }
    }

    else
    {
        printf(RED "ERROR: Unsupported operation\n" RESET);
        return failure;
    }

    return success;
}
