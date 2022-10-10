/*
Name: Anfal
Date: 18/09/2022
Description: LSB IMAGE STEGANOGRAPHY
Sample I/P:
Sample O/P:
*/
#include <stdio.h>
#include "encode.h"
#include "types.h"
#include"common.h"
#include <string.h>

/* Function Definitions */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    printf("INFO    : Read and validation\n");
    // check user has passed .bmp file or not
    // check .txt file is passed or not
    // checking 4th arg is passed or not
    if ((strcmp(strstr(argv[2], ".bmp"),".bmp") == 0))
    {
        encInfo->src_image_fname = argv[2] ;
    }
    else
    {
        return e_failure;
    }
    if ((strcmp(strstr(argv[3], ".txt"),".txt") == 0))
    {
        encInfo->secret_fname = argv[3] ;
    }
    else
    {
        return e_failure;
    }

    if(argv[4] != NULL)
    {
        encInfo-> stego_image_fname = argv[4] ;
    }
    else
    {
        encInfo-> stego_image_fname = "stego.bmp";
    }
    return e_success;
}

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);
 
    // Return image capacity
    return width * height * 3;
}
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_SET);
    fseek(fptr,0,SEEK_END );
    return ftell(fptr);
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    printf("INFO    : Opening files\n");
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}
Status check_capacity(EncodeInfo *encInfo)
{
    printf("INFO    : Check capacity\n");
    // check the capacity od image file bmp > 16+32+32+32+54+ size of secrete file
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    printf("%u\n", encInfo->image_capacity);
    encInfo-> size_secret_file = get_file_size(encInfo->fptr_secret);
    printf("%lu\n", encInfo->size_secret_file);

    if(encInfo->image_capacity > (54 + 16 + 32 + 32 + 32 + (8 * encInfo->size_secret_file)))
    {
        return e_success;
    }
    else
        return e_failure;
}
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    printf("INFO    : copy bmp header \n");
    // copying the 54 byte of header
    char  str[54];
    fseek(fptr_src_image , 0, SEEK_SET);
    fread(str,sizeof(char), 54 ,fptr_src_image);
    fwrite(str,sizeof(char), 54,fptr_dest_image);
    return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    printf("INFO    : Encode magic string \n");
    encode_data_to_image( (char*)magic_string, strlen(magic_string),encInfo->fptr_src_image,encInfo->fptr_stego_image);
    return e_success;

}
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char str[8];
    int i;
    for(i=0; i<size; i++)
    {
        fread(str, sizeof(char),8,fptr_src_image);
        encode_byte_to_lsb(data[i],str);
        fwrite(str, sizeof(char), 8,fptr_stego_image);
    }
    return e_success;
}
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    unsigned mask = 1 << 7;
    int i;
    for(i=0; i<8; i++)
    {
    image_buffer[i] = (image_buffer[i] & 0xFE) | ((data & mask) >> (7 - i));
    mask = mask >> 1;
    }
    return e_success;
    
}
Status encode_secret_file_extn_size(int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    printf("INFO    : Encode file extension size \n");
    char str[32];
    fread(str, 32, 1, fptr_src_image);
    encode_size_to_lsb(str, size);
    fwrite(str, 32, 1, fptr_stego_image);
    return e_success;
}
Status encode_size_to_lsb(char *buffer, int size)
{
    unsigned mask = 1 << 31;
    int i;
    for(i=0; i<32; i++)
    {
    buffer[i] = (buffer[i] & 0xFE) | ((size & mask) >> (31 - i));
    mask = mask >> 1;
    }
    return e_success;
}
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    printf("INFO    : Encode secrete file extension \n");

    encode_data_to_image((char*)file_extn, strlen(file_extn),encInfo->fptr_src_image,encInfo->fptr_stego_image);
    return e_success;

}
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    printf("INFO    : Encode secrete file size \n");
    char str[32];
    fread(str, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(str, file_size);
    fwrite(str, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    printf("INFO    : Encode secrete file data\n");
    char ch;
    int i;
    fseek(encInfo->fptr_secret, 0 , SEEK_SET);
    for(i=0; i< encInfo->size_secret_file; i++)
    {
    fread(&ch, 1,1,encInfo->fptr_secret);
    fread(encInfo->image_data,8,1,encInfo->fptr_src_image);
    encode_byte_to_lsb(ch,encInfo->image_data);
    fwrite(encInfo->image_data,8,1,encInfo->fptr_stego_image);
    }
    return e_success;
}
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    printf("INFO    : Copy remaining image data\n");
    char ch;
    while(fread(&ch,1,1,fptr_src) > 0)
    {
        fwrite(&ch,1,1,fptr_dest);
    }
    return e_success;

}
Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_success)
    {
        printf("Success : Open files successful\n");
        if (check_capacity(encInfo) == e_success)
        {
            printf("Success : Check capacity successful\n");
            if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
            {
                printf("Success : Copy bmp header successful\n");
                if(encode_magic_string(MAGIC_STRING,encInfo) == e_success)
                {
                    printf("Success : Encoding magic string successful\n");
                    if(encode_secret_file_extn_size(4,encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success )
                    {
                        printf("Success : Encoding secrete file extension size successful\n");

                        strcpy(encInfo->extn_secret_file ,strstr(encInfo->secret_fname, "."));
                        if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            printf("Success : Encoding secrete file extension successful\n");

                            if(encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("Success : Encoding secrete file size successful\n");
                                if(encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("success : Encoding secrete file data successful\n");
                                    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                                    {
                                        printf("Success : Copy remaining img data successfful\n");
                                    }
                                    else
                                    {
                                        printf("Failes : Copy remainig image data\n");
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("Failes : Encode secrete file data\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("Success : Encoding secrete file size\n");
                                return e_failure;
                            }
                        }
                        else
                        {
                           
                            printf("Success : Secrete file extension\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Success : Secrete file extension size\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("Success : Magic string encoding\n");
                    return e_failure;
                }
            }
            else
            {
                printf("Failes : Copy bmp header\n");
                return e_failure;
            }
        }
        else
        {
            printf("Failes : Check capacity\n");
            return e_failure;
        }
    }
    else
    {
        printf("failes : Open files \n");
        return e_failure;
    }
}
