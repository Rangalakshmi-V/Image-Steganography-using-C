
#include <stdio.h>
#include <string.h>
#include "decode.h"


/* Text colors */
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define RESET   "\033[0m"


Status validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo)
{
    // Check the stego image extension
    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf(RED "ERROR: Source image must be a BMP file\n" RESET);
        return failure;
    }

    // Store the stego image filename
    strcpy(decInfo->stego_name, argv[2]);

    // Check whether the output filename is provided
    if (argc == 4)
    {
        strcpy(decInfo->output_secret_fname, argv[3]);
    }
    else
    {
        strcpy(decInfo->output_secret_fname, "decoded");
    }

    return success;
}

// Open the stego image file for reading
Status open_stego_image_file(DecodeInfo *decInfo)
{
    // Open the image in read mode
    decInfo->stego_image = fopen(decInfo->stego_name, "r");

    // Check whether the image was opened successfully
    if (decInfo->stego_image == NULL)
    {
        perror("fopen");
        return failure;
    }

    // Display a message after successfully opening the image
    printf("Opened %s successfully\n", decInfo->stego_name);

    return success;
}

/* Skip the 54-byte BMP header to reach the image data */
Status skip_bmp_header(DecodeInfo *decInfo)
{
    // Move the file pointer 54 bytes from the beginning
    if (fseek(decInfo->stego_image, 54, SEEK_SET) != 0)
    {
        printf(RED "ERROR: Unable to skip BMP header\n" RESET);
        return failure;
    }

    // Header skipped successfully
    printf("BMP header skipped successfully\n");

    return success;
}

// Decode one character from 8 image bytes
char decode_byte_from_lsb(char *buffer)
{
    char data = 0;
    int i;

    // Extract one LSB from each image byte
    for (i = 0; i < 8; i++)
    {
        // Shift the existing bits left
        data = data << 1;

        // Add the extracted LSB to data
        data = data | (buffer[i] & 1);
    }

    // Return the reconstructed character
    return data;
}

int decode_size_from_lsb(char *buffer)
{
    int size = 0;
    int i;

    // Extract and combine 32 hidden bits
    for (i = 0; i < 32; i++)
    {
        size = size << 1;
        size = size | (buffer[i] & 1);
    }

    return size;
}

// Decode and verify the magic string
Status decode_magic_string(DecodeInfo *decInfo)
{
    char buffer[8];
    char decoded_char;
    int i;
    int magic_len = strlen(MAGIC_STRING);

    // Decode each character of the magic string
    for (i = 0; i < magic_len; i++)
    {
        // Read 8 image bytes
        if (fread(buffer, 1, 8, decInfo->stego_image) != 8)
        {
            return failure;
        }

        // Extract one character
        decoded_char = decode_byte_from_lsb(buffer);

        // Compare with the expected character
        if (decoded_char != MAGIC_STRING[i])
        {
            printf("ERROR: Magic string does not match\n");
            return failure;
        }
    }

    printf("Magic string matched successfully\n");
    return success;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo, int *size)
{
    char buffer[32];
    int i;

    // Read 32 image bytes to recover the 32-bit size
    if (fread(buffer, 32, 1, decInfo->stego_image) != 1)
    {
        printf(RED "ERROR: Unable to read extension size\n" RESET);
        return failure;
    }

    // Reconstruct the size from the LSBs
    *size = decode_size_from_lsb(buffer);

    // Display the decoded extension size
    printf("Extension size: %d\n", *size);

    return success;
}

// Decode the secret file extension
Status decode_secret_file_extn(DecodeInfo *decInfo, int extn_size)
{
    char buffer[8];
    char ch;
    int i;

    for (i = 0; i < extn_size; i++)
    {
        if (fread(buffer, 1, 8, decInfo->stego_image) != 8)
        {
            printf(RED "ERROR: Unable to read extension\n" RESET);
            return failure;
        }

        ch = decode_byte_from_lsb(buffer);

        // Store the decoded extension
        decInfo->secret_extn[i] = ch;
    }

    // End the string
    decInfo->secret_extn[extn_size] = '\0';

    printf("Decoded extension: %s\n", decInfo->secret_extn);

    return success;
}


/* Decode the secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo, int *file_size)
{
    char buffer[32];

    // Read 32 image bytes containing the hidden file size
    if (fread(buffer, 1, 32, decInfo->stego_image) != 32)
    {
        printf("ERROR: Unable to read secret file size\n");
        return failure;
    }

    // Decode the 32 hidden bits into an integer
    *file_size = decode_size_from_lsb(buffer);

    // Display the decoded file size
    printf("Secret file size: %d bytes\n", *file_size);

    return success;
}


/* Decode the secret data and write it into a file */
Status decode_secret_data(DecodeInfo *decInfo, int file_size)
{
    char buffer[8];
    char ch;
    int i;

    // Open the output file in write mode
    decInfo->secret_fname = fopen(decInfo->output_secret_fname, "w");

    if (decInfo->secret_fname == NULL)
    {
        perror("fopen");
        return failure;
    }

    // Decode each byte of the secret data
    for (i = 0; i < file_size; i++)
    {
        // Read 8 image bytes for one secret byte
        if (fread(buffer, 1, 8, decInfo->stego_image) != 8)
        {
            printf("ERROR: Unable to read secret data\n");
            fclose(decInfo->secret_fname);
            return failure;
        }

        // Reconstruct one character
        ch = decode_byte_from_lsb(buffer);

        // Write the character into the output file
        if (fputc(ch, decInfo->secret_fname) == EOF)
        {
            printf("ERROR: Unable to write secret data\n");
            fclose(decInfo->secret_fname);
            return failure;
        }
    }

    // Close the output file
    fclose(decInfo->secret_fname);

    printf(GREEN "Secret data decoded successfully\n" RESET);

    return success;
}



// Perform the decoding operation
Status do_decoding(DecodeInfo *decInfo)
{
    //step 1 Open the stego image before decoding
    if (open_stego_image_file(decInfo) == failure)
    {
        return failure;
    }
    // Step 2: Skip the BMP header
    if (skip_bmp_header(decInfo) == failure)
    {
        fclose(decInfo->stego_image);
        return failure;
    }
    // Step 3: Decode and check the magic string
    if (decode_magic_string(decInfo) == failure)
    {
    fclose(decInfo->stego_image);
    return failure;
    }

    // Step 4: Decode the secret file extension size
    int extn_size;

    if (decode_secret_file_extn_size(decInfo, &extn_size) == failure)
    {
    fclose(decInfo->stego_image);
    return failure;
    }
    
    
    // Step 5: Decode the secret file extension
    if (decode_secret_file_extn(decInfo, extn_size) == failure)
    {
    fclose(decInfo->stego_image);
    return failure;
    }
    // Then concatenate the filename and extension
    strcpy(decInfo->output_secret_fname, "decoded");
    strcat(decInfo->output_secret_fname, decInfo->secret_extn);

    printf("Output filename: %s\n", decInfo->output_secret_fname);

    
    // Step 6: Decode the secret file size
    int file_size;

    if (decode_secret_file_size(decInfo, &file_size) == failure)
    {
        fclose(decInfo->stego_image);
        return failure;
    }

    // Step 7: Decode the secret data
    if (decode_secret_data(decInfo, file_size) == failure)
    {
    fclose(decInfo->stego_image);
    return failure;
    }
    // Close the image after the current operation
    fclose(decInfo->stego_image);

    return success;
}

