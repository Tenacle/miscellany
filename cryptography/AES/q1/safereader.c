#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

FILE *openFile(char *fileName, char *type)
{
    FILE *fptr = fopen(fileName, type);
    if(fptr == NULL)
    {
        fprintf(stderr, "Error in opening %s\n", fileName);
        exit(1);
    }
    return fptr;
}

// scanf is for stdinput
// fscanf is for files
// sscanf is for strings

int tokenizeAsHex(uint8_t *ret, size_t size, FILE *file)
{
    char line[256] = "";
    char buffer[1028] = "";
    //fprintf(stderr, "Size of buffer: %lu\n", sizeof(buffer));
    size_t counter = 0;
    while(fgets(line, sizeof(line), file) != NULL) // get a line first
    {
        strtok(line, "\n");
        char *tok = strtok(line, " ");
        while(tok != NULL)
        {
            counter++;
            strcat(buffer, tok);
            tok = strtok(NULL, " ");
        }
    }
    //fprintf(stderr, "\nSize: %zu, Buffer: %s\n", counter, buffer);

    // check if file size is the same as the specified one
    if(counter != size)
    {
        fprintf(stderr, "Data byte count is not equal to expected size. Wrong file/config?\n");
        exit(1);
    }

    // convert to hex values (unsigned byte vals)
    uint8_t hex[counter];
    for(size_t i=0; i<counter; i++)
    {
        sscanf(buffer+(i*2), "%2hhx", &hex[i]);
        //fprintf(stderr, "%02X ", hex[i]);
    }
    //fprintf(stderr, "\n");
    //fprintf(stderr, "Size of hex: %lu\n", sizeof(hex));

    // cpy values and prep for return
    if(ret != NULL)
    {
        memmove(ret, hex, sizeof(hex));
    }
    else
    {
        fprintf(stderr, "Please send a valid malloc-ed parameter to put data into");
        exit(1);
    }

    return 0;
}





