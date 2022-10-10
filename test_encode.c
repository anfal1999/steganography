/*
Name: Anfal
Date: 18/09/2022
Description: LSB IMAGE STEGANOGRAPHY
Sample I/P:
Sample O/P:
*/
#include"common.h"
#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "decode.h"

int main(int argc, char *argv[])   // command line arguments
{

    if (check_operation_type(argv) == e_encode)  
    {
        printf("Choosen Encoding\n");
        EncodeInfo encInfo;
        if(read_and_validate_encode_args(argv,&encInfo) == e_success)
        {
            printf("Success : Read and validate successful\n");
            if ( do_encoding(&encInfo) == e_success)
            {
                printf("Started Encoding Process..\n");
            }
            else
            {
            printf("Encoding Process...Terminates\n");
            return e_failure;
            }
        }
        else
        {
            printf("Failes : Read and validate\n");
            return e_failure;
        }

    }

    else if (check_operation_type(argv) == e_decode)

    {   
        printf("Choosen Decoding\n");
        DecodeInfo decInfo;
        if(read_and_validate_decode_args(argv,&decInfo) == d_success)
        {
            printf("Success : Decode read and validate successful\n");
            if(do_decoding(&decInfo) == d_success)
            {
                printf("Success : Decoding successful\n");
            }
            else
            {
                return d_failure;
            }
        }
        else
        {
            return d_failure;
        }
    }

     
    else
    {
        return d_failure;
    }
    return 0;
}

OperationType check_operation_type(char *argv[])
{
    // printf("%s", argv[0]);


    if ((strcmp(argv[1], "-e")) == 0) // strcmp(argv[1],)
        return e_encode;

    else if ((strcmp(argv[1], "-d")) == 0)
        return e_decode;
    else
        return e_unsupported;
}
