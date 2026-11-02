
#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "common.h"

typedef struct DecodeInfo
{
    char stego_name[30];
    FILE *stego_image;
    char output_secret_fname[30];
    FILE *secret_fname;
    char secret_extn[10];
} DecodeInfo;

Status validate_decode_args(int argc,char *argv[], DecodeInfo *decInfo);
Status do_decoding(DecodeInfo *decInfo);
Status open_stego_image_file(DecodeInfo *decInfo);
Status skip_bmp_header(DecodeInfo *decInfo);
char decode_byte_from_lsb(char *buffer);
Status decode_magic_string(DecodeInfo *decInfo);
int decode_size_from_lsb(char *buffer);
Status decode_secret_file_extn(DecodeInfo *decInfo, int extn_size);
Status decode_secret_file_size(DecodeInfo *decInfo, int *file_size);
Status decode_secret_data(DecodeInfo *decInfo, int file_size);

#endif
