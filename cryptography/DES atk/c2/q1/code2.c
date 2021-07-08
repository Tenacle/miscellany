#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "safereader.h"
#define BYTE_SIZE 16
#define SBOX_BYTE_SIZE 256
#define SBOX_SIZE 16
#define S_SIZE 4

// NEW STUFF
#define BLOCK_SIZE 64       // bits
#define ROUNDS 16           // bits
#define KEY_SIZE 56         // bits



typedef struct Class
{
    int var;
} Class_Alias;

typedef enum{false, true} bool;
typedef enum{left, right} direction;

uint8_t *hexFileHandler(size_t size, char *fileName);
int printHex(uint8_t *toPrint, size_t size);
uint8_t **mallocMatrix(size_t mSize);



//uint8_t **toMatrix(uint8_t *array, size_t size, size_t mSize);
uint8_t **toState(uint8_t *array, size_t mSize);
uint8_t *fromState(uint8_t **matrix, size_t mSize);
int printMatrix(uint8_t **toPrint, size_t mSize);
int stderrPrintMatrix(uint8_t **toPrint, size_t mSize);
int freeMatrix(uint8_t **matrix, size_t mSize);

// bit operations
int xorMatrix(uint8_t **ans, uint8_t **a, uint8_t **b, size_t mSize);
int shift(direction direc, size_t steps, uint8_t *arr, size_t mSize);

// stages
int addRoundKey(uint8_t **s, uint8_t **subKey, size_t size);
int subByte(uint8_t **s, size_t sSize, uint8_t *sbox);
int subByteArr(uint8_t *arr, size_t size, uint8_t *sbox);
int shiftRow(direction direc, uint8_t **s, size_t mSize);
int mixColumns(uint8_t **s, size_t mSize);
int invMixColumns(uint8_t **s, size_t mSize);

// expand
uint8_t *genKeySched(uint8_t *key, size_t byteSize, uint8_t *sbox);
int keyWordHandler(uint8_t *word, size_t round, uint8_t *sbox);

// global vars
uint8_t RCON[10] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};

// NEW STUFF
// TODO Substitution permutation network


int main(int argc, char *argv[])
{
    fprintf(stderr, "\n*** Starting ***\n");
    if(argc != 5)
    {
        fprintf(stderr, "Expecting 4 parameters: <plaintext, key, sbox, sbox_inv>\n");
        exit(1);
    }

    // *** OPEN FILES ***
    //fprintf(stderr, "\nReading key file\n");
    uint8_t *key = hexFileHandler(BYTE_SIZE, argv[2]);
    //free(key);
    //printMatrix(k, S_SIZE);

    //fprintf(stderr, "\nReading plaintext file\n");
    uint8_t *in = hexFileHandler(BYTE_SIZE, argv[1]);
    uint8_t **s = toState(in, S_SIZE);
    free(in);
    //printMatrix(s, S_SIZE);

    //fprintf(stderr, "\nReading sbox\n");
    //uint8_t *sboxHex = hexFileHandler(SBOX_BYTE_SIZE, argv[1]);
    //uint8_t **sbox = toMatrix(sboxHex, SBOX_BYTE_SIZE, SBOX_SIZE);
    //free(sboxHex);
    uint8_t *sbox = hexFileHandler(SBOX_BYTE_SIZE, argv[3]);
    uint8_t *sboxInv = hexFileHandler(SBOX_BYTE_SIZE, argv[4]);
    // *** END OPEN FILES ***

    // TODO: Encrypt
    size_t nR = 10;     // number of rounds
    size_t nB = 4;
    size_t nK = 4;
    size_t round = 0;
    uint8_t *keySched = genKeySched(key, BYTE_SIZE, sbox);
    uint8_t **k = toState(keySched+(round*16), S_SIZE);
    fprintf(stderr, "nR: %lu\nnB: %lu\nnK: %lu\n", nR, nB, nK);
    fprintf(stderr, "*** state ***\n");
    stderrPrintMatrix(s, S_SIZE);
    fprintf(stderr, "*** key sched ***\n");
    printHex(keySched, 176);

    // *** Four stages ***
    // 1) addRoundKey
    fprintf(stderr, "*** (1): addRoundKey ***\n");
    addRoundKey(s, k, S_SIZE);
    stderrPrintMatrix(s, S_SIZE);
    fprintf(stderr, "***\n");
    addRoundKey(s, k, S_SIZE);
    stderrPrintMatrix(s, S_SIZE);

    // 2) subByte
    fprintf(stderr, "*** (2): subByte ***\n");
    subByte(s, S_SIZE, sbox);
    stderrPrintMatrix(s, S_SIZE);
    fprintf(stderr, "***\n");
    subByte(s, S_SIZE, sboxInv);
    stderrPrintMatrix(s, S_SIZE);

    // 3) shiftRow
    fprintf(stderr, "*** (3): shiftRow ***\n");
    shiftRow(left, s, S_SIZE);
    stderrPrintMatrix(s, S_SIZE);
    fprintf(stderr, "***\n");
    shiftRow(right, s, S_SIZE);
    stderrPrintMatrix(s, S_SIZE);

    // 4) mixColumns
        //addRoundKey(s, k, S_SIZE);
    fprintf(stderr, "*** (4): mixColumns ***\n");
    mixColumns(s, S_SIZE);
    stderrPrintMatrix(s, S_SIZE);
    fprintf(stderr, "***\n");
    invMixColumns(s, S_SIZE);
    stderrPrintMatrix(s, S_SIZE);

    for(size_t round=0; round<nR; round++)
    {
        if(round != nR-1)
        {
            // mix columns here.
        }
    }

    // TODO: Decrypt

    // TODO: Cleanup
    freeMatrix(k, S_SIZE);
    freeMatrix(s, S_SIZE);
    free(sbox);
    free(sboxInv);
    //freeMatrix(sbox, SBOX_SIZE);

    // Done
    fprintf(stderr, "\n*** Done ***\n");
    exit(0);
}

int invMixColumns(uint8_t **s, size_t mSize)
{
    // https://cboard.cprogramming.com/c-programming/87805-[tutorial]-implementing-advanced-encryption-standard.html
    uint8_t col[mSize];
    for(size_t c=0; c<mSize; c++)
    {
        col[0] = (0x0E*s[0][c])+(0x0B*s[1][c])+(0x0D*s[2][c])+(0x09*s[3][c]);
        col[1] = (0x09*s[0][c])+(0x0E*s[1][c])+(0x0B*s[2][c])+(0x0D*s[3][c]);
        col[2] = (0x0D*s[0][c])+(0x09*s[1][c])+(0x0E*s[2][c])+(0x0B*s[3][c]);
        col[3] = (0x0B*s[0][c])+(0x0D*s[1][c])+(0x09*s[2][c])+(0x0E*s[3][c]);
        
        s[0][c] = col[0];
        s[1][c] = col[1];
        s[2][c] = col[2];
        s[3][c] = col[3];
    }
    return 0;
}

int mixColumns(uint8_t **s, size_t mSize)
{
    // https://cboard.cprogramming.com/c-programming/87805-[tutorial]-implementing-advanced-encryption-standard.html
    uint8_t col[mSize];
    for(size_t c=0; c<mSize; c++)
    {
        col[0] = (0x02*s[0][c])+(0x03*s[1][c])+(0x01*s[2][c])+(0x01*s[3][c]);
        col[1] = (0x01*s[0][c])+(0x02*s[1][c])+(0x03*s[2][c])+(0x01*s[3][c]);
        col[2] = (0x01*s[0][c])+(0x01*s[1][c])+(0x02*s[2][c])+(0x03*s[3][c]);
        col[3] = (0x03*s[0][c])+(0x01*s[1][c])+(0x01*s[2][c])+(0x02*s[3][c]);
        
        s[0][c] = col[0];
        s[1][c] = col[1];
        s[2][c] = col[2];
        s[3][c] = col[3];
    }
    return 0;
}

int keyWordHandler(uint8_t *word, size_t round, uint8_t *sbox)
{
    shift(left, 1, word, 4);
    subByteArr(word, 4, sbox);
    word[0] = word[0] ^ RCON[round];
    return 0;
}

uint8_t *genKeySched(uint8_t *key, size_t byteSize, uint8_t *sbox)
{
    // expandedSize = (nR + 1 ) * size
    // TODO:
    uint8_t *keySched = malloc(176);
    if(keySched == NULL)
    {
        fprintf(stderr, "Malloc failed.\n");
        exit(1);
    }
    size_t currByteSize = 0;
    size_t round = 1;
    uint8_t word[4] = {0};

    for(size_t i=0; i<byteSize; i++)
    {
        keySched[i] = key[i];
        fprintf(stderr, "[%02x] ", key[i]);
    }
    fprintf(stderr, "\n");
    currByteSize += byteSize;

    while(currByteSize < 176)
    {
        for(size_t i=0; i<4; i++)
        {
            word[i] = keySched[(currByteSize-4) + i];
        }

        if(currByteSize % byteSize == 0)
        {
            keyWordHandler(word, round++, sbox);
        }

        for(size_t i=0; i<4; i++)
        {
            keySched[currByteSize] = keySched[currByteSize-byteSize] ^ word[i];
            currByteSize++;
        }
    }

    return keySched;
}

int subByteArr(uint8_t *arr, size_t size, uint8_t *sbox)
{
    for(size_t i=0; i<size; i++)
    {
        arr[i] = sbox[arr[i]];
    }
    return 0;
}

int subByte(uint8_t **s, size_t sSize, uint8_t *sbox)
{
    for(size_t r=0; r<sSize; r++)
    {
        for(size_t c=0; c<sSize; c++)
        {
            s[r][c] = sbox[s[r][c]];
        }
    }
    return 0;
}

int shiftRow(direction direc, uint8_t **s, size_t mSize)
{
    for(size_t r=1; r<mSize; r++)
    {
        shift(direc, r, s[r], mSize);
    }
    return 0;
}
int shift(direction direc, size_t steps, uint8_t *arr, size_t mSize)
{
    uint8_t helper[mSize];
    for(size_t i=0; i<mSize; i++)
    {
        if(direc)
        {
            helper[(i+steps)%mSize] = arr[i];
        }
        else
        {
            helper[(i-steps)%mSize] = arr[i];
        }
    }
    for(size_t i=0; i<mSize; i++)
    {
        arr[i] = helper[i];
    }
    return 0;
}

int addRoundKey(uint8_t **s, uint8_t **subKey, size_t size)
{
    // update state by XOR w/ sub-key (TODO: generate sub-keys)
    xorMatrix(s, s, subKey, size);
    return 0;
}
int xorMatrix(uint8_t **ret, uint8_t **val0, uint8_t **val1, size_t size)
{
    for(size_t r=0; r<size; r++)
    {
        for(size_t c=0; c<size; c++)
        {
            ret[r][c] = val0[r][c] ^ val1[r][c];
        }
    }
    return 0;
}

uint8_t **toState(uint8_t *array, size_t mSize)
{
    uint8_t **matrix = mallocMatrix(mSize);
    for(size_t r=0; r<mSize; r++)
    {
        for(size_t c=0; c<mSize; c++)
        {
            matrix[r][c] = array[r + (4*c)];
            //fprintf(stderr, "%02x ", matrix[r][c]);
        }
        //fprintf(stderr, "\n");
    }
    return matrix;
}
uint8_t *fromState(uint8_t **matrix, size_t mSize)
{
    uint8_t *array = malloc(mSize * mSize);
    for(size_t r=0; r<mSize; r++)
    {
        for(size_t c=0; c<mSize; c++)
        {
            array[r + (4*c)] = matrix[r][c];
            //fprintf(stderr, "%02x ", matrix[r][c]);
        }
        //fprintf(stderr, "\n");
    }
    return array;
}

//uint8_t **toMatrix(uint8_t *array, size_t size, size_t mSize)
//{
//    // https://www.geeksforgeeks.org/dynamically-allocate-2d-array-c/
//    uint8_t **matrix = mallocMatrix(mSize);
//    size_t i = 0;
//    for(size_t r=0; r<mSize; r++)
//    {
//        if(matrix[r] == NULL)
//        {
//            fprintf(stderr, "Malloc failed.\n");
//            exit(1);
//        }
//        uint8_t *elem = {0};
//        for(size_t c=0; c<mSize; c++)
//        {
//            elem = array[i++];
//            matrix[r][c] = elem;
//        }
//        //fprintf(stderr, "\n");
//    }
//    return matrix;
//}
int stderrPrintMatrix(uint8_t **toPrint, size_t mSize)
{
    for(size_t r=0; r<mSize; r++)
    {
        for(size_t c=0; c<mSize; c++)
        {
            printf("[%02x] ", toPrint[r][c]);
        }
        printf("\n");
    }
    return 0;
}

int printMatrix(uint8_t **toPrint, size_t mSize)
{
    for(size_t r=0; r<mSize; r++)
    {
        for(size_t c=0; c<mSize; c++)
        {
            printf("[%02x] ", toPrint[r][c]);
        }
        printf("\n");
    }
    return 0;
}

uint8_t **mallocMatrix(size_t mSize)
{
    uint8_t **matrix = malloc(mSize * sizeof(uint8_t *));
    if(matrix == NULL)
    {
        fprintf(stderr, "Malloc failed.\n");
        exit(1);
    }
    for(size_t r=0; r<mSize; r++)
    {
        matrix[r] = malloc(mSize * sizeof(uint8_t));
    }
    return matrix;
}

int freeMatrix(uint8_t **matrix, size_t mSize)
{
    for(size_t r=0; r<mSize; r++)
    {
        free(matrix[r]);
    }
    free(matrix);
    return 0;
}

int printHex(uint8_t *toPrint, size_t size)
{
    for(size_t i=0; i<size; i++)
    {
        printf("%02X ", toPrint[i]);
    }
    printf("\n");
    return 0;
}

uint8_t *hexFileHandler(size_t size, char *fileName)
{
    // https://stackoverflow.com/questions/19106263/convert-string-array-into-hex-numbers-in-c
    FILE *file = openFile(fileName, "r");
    uint8_t *data = NULL;
    data = malloc(size);
    tokenizeAsHex(data, size, file);
    fclose(file);
    return data;
}

