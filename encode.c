#include <stdio.h>
#include <string.h>
#include "common.h"
#include "encode.h"

/* Text colors */
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define RESET   "\033[0m"


//check operation
 Opr_type check_operation(char *option)
{
    if (strcmp(option, "-e") == 0)
    {
        return encode;
    }
    else if (strcmp(option, "-d") == 0)
    {
        return decode;
    }
    else
    {
        return unsupported;
    }
}

Status validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // 1. Check source image extension
    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf(RED "Error: Source image must be a .bmp file\n" RESET);
        return failure;
    }
    printf("Src file is .bmp\n");

    encInfo->src_image_fname = argv[2];

    // 2. Store secret filename
    encInfo->secret_fname = argv[3];

    // 3. Check secret file extension
    char *ext = strrchr(argv[3], '.');

    if (ext == NULL ||
        (strcmp(ext, ".txt") != 0 &&
         strcmp(ext, ".c") != 0 &&
         strcmp(ext, ".sh") != 0))
    {
        printf(RED "Error: Please pass a valid secret file (.txt, .c, .sh)\n" RESET);
        return failure;
    }

    // 4. Store secret file extension
    strcpy(encInfo->secret_extn, ext);
    encInfo->secret_extn_size = strlen(ext);

    // 5. Set output image filename
    if (argv[4] != NULL)
    {
        encInfo->output_image_fname = argv[4];
    }
    else
    {
        encInfo->output_image_fname = "stego.bmp";
    }

    return success;
}

/* High-level function to perform all encoding steps */
Status do_encoding(char *agrv[], EncodeInfo *encInfo)
{
    // Step 1: Open all required files
    printf(GREEN "Opening required files\n" RESET);

    if (open_files(encInfo) == failure)
    {
        printf(RED "ERROR: Opening files failed\n" RESET);
        return failure;
    }

    printf("Done\n");

    // Print the image capacity in bytes
    printf("%u\n", get_image_size_for_bmp(encInfo->src_image_fptr));
    // Calculate the secret file size
    encInfo->secret_file_size = get_file_size(encInfo->secret_fptr);

    // Step 2: Check the secret file size
    printf("%lu\n", encInfo->secret_file_size);
    printf(YELLOW "Checking for %s size\n" RESET, encInfo->secret_fname);

    if (encInfo->secret_file_size == 0)
    {
        printf(RED "Error: Secret file is empty\n" RESET);
        return failure;
    }

    printf("Done. Not Empty\n");

    // Step 3: Check whether the image can hold the secret data
    printf(YELLOW "Checking for %s capacity to handle %s\n" RESET,
           encInfo->src_image_fname,
           encInfo->output_image_fname);

    if (check_capacity(encInfo) == failure)
    {
        printf(RED "Error: Image does not have enough capacity\n" RESET);
        return failure;
    }

    printf("Done. Found Ok\n");

    printf(YELLOW "## Encoding Procedure Started ##\n" RESET);

    // Step 4: Copy the BMP header
    printf("Copying Image Header\n");

    if (copy_bmp_header(encInfo) == failure)
    {
        printf(RED "ERROR: Failed to copy image header\n" RESET);
        return failure;
    }

    printf("Done\n");

    // Step 5: Encode the magic string
    printf("Encoding Magic String Signature\n");

    if (encode_magic_string(MAGIC_STRING, encInfo) == failure)
    {
        printf(RED "ERROR: Failed to encode magic string\n" RESET);
        return failure;
    }

    printf("Done\n");

    // Step 6: Encode the secret file extension
    printf("Encoding %s File Extension\n", encInfo->secret_fname);

    if (encode_secret_file_extn(encInfo) == failure)
    {
        printf(RED "ERROR: Failed to encode file extension\n" RESET);
        return failure;
    }

    printf("Done\n");

    // Step 7: Encode the secret file size
    printf("Encoding %s File Size\n", encInfo->secret_fname);

    char img_buffer[32];

    // Read 32 bytes from the source image
    if (fread(img_buffer, 32, 1, encInfo->src_image_fptr) != 1)
    {
        printf(RED "ERROR: Failed to read image data\n" RESET);
        return failure;
    }

    // Hide the secret file size in the LSBs
    if (encode_4byte_to_lsb(encInfo->secret_file_size,
                            img_buffer) == failure)
    {
        printf(RED "ERROR: Failed to encode file size\n" RESET);
        return failure;
    }

    // Write the modified image bytes to the output image
    if (fwrite(img_buffer, 32, 1,
               encInfo->output_image_fptr) != 1)
    {
        printf(RED "ERROR: Failed to write image data\n" RESET);
        return failure;
    }

    printf("Done\n");

    // Step 8: Encode the secret file data
    printf("Encoding %s File Data\n", encInfo->secret_fname);

    if (encode_secret_file_data(encInfo) == failure)
    {
        printf(RED "ERROR: Failed to encode secret file data\n" RESET);
        return failure;
    }

    printf("Done\n");

    // Step 9: Copy the remaining image data
    printf("Copying Left Over Data\n");

    if (copy_remaining_img_data(encInfo) == failure)
    {
        printf(RED "ERROR: Failed to copy remaining image data\n" RESET);
        return failure;
    }

    printf("Done\n");

    // Step 10: Close all opened files
    fclose(encInfo->src_image_fptr);
    fclose(encInfo->secret_fptr);
    fclose(encInfo->output_image_fptr);

    // Display the final success message
    printf(GREEN "## Encoding Done Successfully ##\n" RESET);

    return success;
}
Status open_files(EncodeInfo *encInfo)
{
    encInfo->src_image_fptr = fopen(encInfo->src_image_fname, "r");
    //open source image
    if (encInfo->src_image_fptr == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, RED "ERROR: Unable to open file %s\n" RESET, encInfo->src_image_fname);

    	return failure;
    }
     printf("Opened %s\n", encInfo->src_image_fname);

    //open secrete file
    encInfo->secret_fptr = fopen(encInfo->secret_fname, "r");

    if (encInfo->secret_fptr == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, RED "ERROR: Unable to open file %s\n" RESET, encInfo->secret_fname);

    	return failure;
    }
    printf("Opened %s\n", encInfo->secret_fname);


    //open output image
    encInfo->output_image_fptr = fopen(encInfo->output_image_fname, "w");

    if (encInfo->output_image_fptr == NULL)
    {
    	perror("fopen");
    	fprintf(stderr,RED "ERROR: Unable to open file %s\n" RESET, encInfo->output_image_fname);

    	return failure;
    }
    printf("Opened %s\n", encInfo->output_image_fname);

    return success;
}


/* Check if the source image capacity is sufficient */
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->secret_file_size = get_file_size(encInfo->secret_fptr);

    unsigned int image_capacity = get_image_size_for_bmp(encInfo->src_image_fptr);

    unsigned long int required_capacity =
        (strlen(MAGIC_STRING) +
         sizeof(int) +
         encInfo->secret_extn_size +
         sizeof(int) +
         encInfo->secret_file_size) * 8;

    if (image_capacity < required_capacity)
    {
        return failure;
    }
    return success;
}

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

unsigned int get_image_size_for_bmp(FILE *fptr_image)
{
    unsigned int width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);


    // Return image capacity
    return width * height * 3;
}
/*
 * Get secret file size
 */
/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: success or failure, on file errors
 */
unsigned int get_file_size(FILE *fptr)
{
    unsigned int size;
     // Move file pointer to the end of the file
    fseek(fptr, 0, SEEK_END);

      // Get the file size
    size = ftell(fptr);

    // Move file pointer back to the beginning
    rewind(fptr);

    return size;
}

/* Copy the BMP header from source image to output image */
Status copy_bmp_header(EncodeInfo *encInfo)
{
    char header[54];

    rewind(encInfo->src_image_fptr);

    fread(header, 54, 1, encInfo->src_image_fptr);

    fwrite(header, 54, 1, encInfo->output_image_fptr);

    return success;
}

/* Store Magic String */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];

    for (int i = 0; magic_string[i] != '\0'; i++)
    {
        fread(buffer, 8, 1, encInfo->src_image_fptr);

        encode_1byte_to_lsb(magic_string[i], buffer);

        fwrite(buffer, 8, 1, encInfo->output_image_fptr);
    }

    return success;
}

/* Encode secret file extension */
Status encode_secret_file_extn(EncodeInfo *encInfo)
{
    char img_buffer[32];

    /* Encode extension size */
    fread(img_buffer, 32, 1, encInfo->src_image_fptr);

    encode_4byte_to_lsb(encInfo->secret_extn_size, img_buffer);

    fwrite(img_buffer, 32, 1, encInfo->output_image_fptr);

    /* Encode extension characters */
    for (int i = 0; i < encInfo->secret_extn_size; i++)
    {
        fread(img_buffer, 8, 1, encInfo->src_image_fptr);

        encode_1byte_to_lsb(encInfo->secret_extn[i], img_buffer);

        fwrite(img_buffer, 8, 1, encInfo->output_image_fptr);
    }

    return success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    char img_buffer[8];

    rewind(encInfo->secret_fptr);

    for (int i = 0; i < encInfo->secret_file_size; i++)
    {
        fread(&ch, 1, 1, encInfo->secret_fptr);

        fread(img_buffer, 8, 1, encInfo->src_image_fptr);

        encode_1byte_to_lsb(ch, img_buffer);

        fwrite(img_buffer, 8, 1, encInfo->output_image_fptr);
    }

    return success;
}

Status encode_1byte_to_lsb(char ch, char *image_bytes)
{
    int get_msb;

    for (int i = 7; i >= 0; i--)
    {
        get_msb = (ch & (1 << i)) >> i;

        image_bytes[7 - i] = image_bytes[7 - i] & (~1);

        image_bytes[7 - i] = image_bytes[7 - i] | get_msb;
    }

    return success;
}

/* Encode 4 byte (int) into LSB of image data array */
Status encode_4byte_to_lsb(int data, char *buffer_32)
{
    int get_msb;

    for (int i = 31; i >= 0; i--)
    {
        get_msb = (data & (1 << i)) >> i;

        buffer_32[31 - i] = buffer_32[31 - i] & (~1);

        buffer_32[31 - i] = buffer_32[31 - i] | get_msb;
    }

    return success;
}

/* Copy remaining image bytes */
Status copy_remaining_img_data(EncodeInfo *encInfo)
{
    char ch;

    while (fread(&ch, 1, 1, encInfo->src_image_fptr) > 0)
    {
        fwrite(&ch, 1, 1, encInfo->output_image_fptr);
    }

    return success;
}

