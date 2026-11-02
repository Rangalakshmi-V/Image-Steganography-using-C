# Image-Steganography-using-C
A C programming project to hide and extract secret data inside BMP images using the LSB technique.
# Image Steganography Using C

## Project Overview
Image Steganography is a technique used to hide secret information inside an image. This project is implemented using C programming and the Least Significant Bit (LSB) technique.

## Objective
To securely hide a secret file inside a BMP image and retrieve the original file through decoding.

## Technologies Used
- C Programming
- File Handling
- Structures and Pointers
- Bitwise Operators
- LSB Technique

## Features
- Encode secret files into BMP images.
- Decode and extract hidden secret information.
- Verify hidden data using a magic string.
- Store and retrieve file extension and file size.

## Encoding Process
1. Validate command-line arguments.
2. Check image capacity and secret file size.
3. Copy the BMP header.
4. Encode the magic string.
5. Encode the secret file extension and size.
6. Encode secret file data using LSB.
7. Generate the stego image.

## Decoding Process
1. Open the stego image.
2. Skip the BMP header.
3. Verify the magic string.
4. Extract the secret file extension and size.
5. Decode the hidden data.
6. Recreate the original secret file.

## Compilation
gcc *.c


## Execution
### Encoding
./a.out -e beautiful.bmp secret.txt stego.bmp

### Decoding
./a.out -d stego.bmp

## Concepts Learned
This project helped me gain practical knowledge of C programming, file handling, structures, pointers, bitwise operations, and image steganography.

