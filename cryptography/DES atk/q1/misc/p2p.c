#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
//#include <iostream>
#include "physical.h"
//#include <math.h>

#define SENTINEL 0x7E
#define PREAMBLE 0xA5
#define POSTAMBLE 0xA6
#define TFRAME 2
#define TDELAY 1    // good value could be TProcess + TACK
                    // Fr#1 will be received at TFRAME + TDELAY
#define TOUT 4      // needs to be greater than expected time to receive frame (> TF + 2TD)
#define WSnd 3      // has to equal (2TD/TF + 1)
#define WRcv 1      // try 1 or 2
                    // send as fast as possible: 0,1,2,3,...

#define MAX_WIN_SIZE 7
#define MAX_INFO_SIZE 1017
// control
#define CTL_SPRV 0xA0    
#define CTL_INFO 0x50    
#define CTL_SS_ACK 0x50
#define CTL_SS_NAK 0xA0
#define CTL_SS_INF 0x90
#define CTL_P 0xA
#define CTL_F 0x5

// list of timeouts for frames
typedef struct DeltaList
{
    int delta;
    int frNum;
    int frSize;
    char *frame;
    struct DeltaList *next;
} FrameList;


typedef struct
{
    int peerID;
    int winSize;
    int lowestACKFr;
    int lastFrSent;
    int buffFrNum;
} Peer;


typedef enum {NORMAL, TIMEOUT} EVENTLIST;


// Global variables
long timeMSec;
PhysicalState *physical;

//clock_t tick;
// clocks:
// https://stackoverflow.com/questions/10192903/time-in-milliseconds-in-c/10192994
// https://stackoverflow.com/questions/3756323/how-to-get-the-current-time-in-milliseconds-from-c-in-linux
// https://linux.die.net/man/3/clock_gettime
// *** printf("%-44s %21f\n", "Time in s + us:", (tick.tv_sec * 1e6) + (tick.tv_nsec / 1e3));                                  // what we'll use for time

/* declarators */
int updateTime(long *clock);
int updateFrameList(FrameList *frameList);
char *createFrame(char *info, uint8_t ss, uint8_t pf, int rFrNum, int sFrNum);
void *freeFrameList(FrameList *frameList);
FrameList *head;
FrameList *tail;
boolean l2rRx = false;

FrameList *createFrameList(int delta, int frNum, char *frame)
{
    FrameList* frameList = malloc(sizeof(FrameList));
    frameList->delta = delta;
    frameList->frNum = frNum;
    frameList->frSize = 4 + strlen(frame+4) + 3;                    // preamble(2B) + ctl(2B) + info + FCS(1B) + postamble(2B)
    //printf("Creating frame with size: %d\n", frameList->frSize);
    frameList->frame = malloc(frameList->frSize);
    for(int i = 0; i < frameList->frSize; i++)
    {
        frameList->frame[i] = frame[i];
    }
    //printf("*****Frame copied: %s*******\n", frameList->frame+4);
    return frameList;
}


// TODO: join the two threaded routines into one

void *routineTx(int id, char *fileName)
{
    char messages[MAX_INFO_SIZE];
    FILE *fptr = fopen(fileName);
    if(fptr == NULL)
    {
        printf(stderr, "Tx: Error in opening file %s\n", fileName);
        exit(1);
    }

    // parse file into frames


}

void *routine_L2R()
{
    // initialize
    char line[MAX_INFO_SIZE];
    FILE *fptr = fopen("./L2RFrames.txt", "r");
    if(fptr == NULL)
    {
        printf("Error! Opening file");
        exit(1);
    }

    printf("Reading file");
    int index = 0;
    // encapsulate each line of text into a template frame (stuffed) and add it to FrameList
    while(fgets(line, MAX_INFO_SIZE, fptr) != NULL)
    {
        char *newFrame = createFrame(line, CTL_SS_INF, CTL_F, 0, index);
        FrameList *tmp = createFrameList(-1, index, newFrame);
        if(head == NULL)
        {
            head = tmp;
            tail = head;
        }
        else
        {
            tail->next = tmp;
            tail = tmp;
        }
        //printf("\t\t\tcreated a fully stuffed frame: *****%s****\n", newFrame+4);
        index++;
        usleep(1);
    }

    //printf("\t\t\tPrinting info of frames in FrameList of size %d\n", index);
    FrameList *curr = head;
    index = 0;
    while(curr != NULL)
    {
        printf("Frame %d: *******%s*******\n", index++, curr->frame+4);
        curr = curr->next;
    }

    FrameList *currFrame = head;
    index = 0;
    while(currFrame != NULL)
    {
        // wait for L2RReady
        pthread_mutex_lock(&physical->L2RLock);
        while(!physical->L2RReady)
        {
            printf("*******************WAITING for Rx to finish\n");
            pthread_cond_wait(&physical->L2RTxSignal, &physical->L2RLock);
        }
        printf("Frame signaled for Transmission\n");

        transmitFrame(L2R, (unsigned char *)currFrame->frame, currFrame->frSize);
        printf("Transmitted frame (%d transmitted)\n", ++index);


        currFrame = currFrame->next;
        pthread_cond_signal(&physical->L2RRxSignal);
        pthread_mutex_unlock(&physical->L2RLock);

    }


    printf("L2R thread done. L2RReady is now: %d\n", physical->L2RReady);
    freeFrameList(head);
    fclose(fptr);
    pthread_exit(NULL);
}

void *routine_R2L()
{
    int deathCounter = 0;

    while(deathCounter < 5)
    {
        // wait for Rx
        //if(!physical->L2RReady)
        pthread_mutex_lock(&physical->L2RLock);
        while(physical->L2RReady)
        {
            printf("*******************WAITING for Tx to finish\n");
            pthread_cond_wait(&physical->L2RRxSignal, &physical->L2RLock);
        }

        // time to receive frames
        //printf("Received a go signal to receive items (%d). Sleeping for a bit..\n", physical->L2RReady);
        printf("Received a go signal to receive items (%d). Sleeping for a bit..\n", physical->L2RReady);
        char uc;
        char c[MAX_FRAME_SIZE] = {0};
        int frameIndex = 0;
        printf("Receiving\n");
        while(true)
        //for(int i = 0; i < 100; i++)
        {
            uc = (char)receiveByte(L2R);
            c[frameIndex++] = uc;
            if(c[frameIndex] == 0xA6 && c[frameIndex-1] == 0x7E)
            {
                break;
            }
            if(frameIndex > 1025)
            {
                break;
            }
        }
        printf("Stopped receiving at index %d. L2RReady is now: %d\n", frameIndex, physical->L2RReady);

        l2rRx = false;
        pthread_cond_signal(&physical->L2RTxSignal);
        pthread_mutex_unlock(&physical->L2RLock);

        printf("receieved frame: %s\n", c+4);
        deathCounter++;
    }


    //transmitFrame(L2R, (unsigned char *)0xFFFFFFFF, 8);
    printf("R2L done. value of L2RReady   : %d\n", physical->L2RReady   );     // true == ready to transmit, false == bytes to receive
    return NULL;
}

int main()
{
    // initialize
    EVENTLIST event = NORMAL;
    updateTime(&timeMSec);
    physical = initPhysical();
    pthread_t tL2R, tR2L;
    head = NULL;
    tail = NULL;

    pthread_create(&tL2R, NULL, &routine_L2R, NULL);
    pthread_create(&tR2L, NULL, &routine_R2L, NULL);

    pthread_join(tL2R, NULL);
    pthread_join(tR2L, NULL);

    printf("Simulation done\n");
    cleanPhysical();
}

// create new Peer to send/recv frames from/to
Peer *initPeer(char *fileToSend, int peerID)
{
    int textSize = strlen(fileToSend);
    return NULL;
}

int updateFrameList(FrameList *head)
{
    updateTime(&timeMSec);
//    printf("%-12s %12ld\n", "Curr time:", timeMSec);                                  // what we'll use for time

    // TODO: process frames if there are any
//    FrameList *currFrameList = head;
//    while(currFrameList != NULL)
//    {
//        printf("Frame found\n");
//        //printf("currFrame: %p\n", currFrameList);
//        // TODO: timeout detected
//            //  - TODO: check if frame has ACK
//            //if timedOut <= 100 || timedOut >= -100
//    }


    /* 
     * - head->delta -= clock                 // decrement clocks
     * - head->delta <= 0: eType = timeout    // prep timeout event
     * - eParm = head->frNum
     * - head = head->next
     * - causeEvent(eType, eParm)               // trigger/fire event
     */
    return 0;
}

// update global var timeMSec
int updateTime(long *clock)
{
    struct timespec tick;
    clock_gettime(CLOCK_MONOTONIC, &tick);                                              // advance time
    long nsec = tick.tv_nsec;
    if(nsec > 0)
    {
        nsec = nsec/1000000;
    }
    else if(nsec < 0)
    {
        printf("FATAL ERROR WHAT IS THIS?\n");
        exit(1);
    }
    *clock = (tick.tv_sec * 1000) + nsec;
    printf("Time: %lu\n", *clock);
    return 0;
}


uint16_t createControl(uint8_t ss, uint8_t pf, int rFrNum, int sFrNum)
{
    uint16_t control;
    uint8_t frNum;
    if(ss == CTL_SS_INF)            // normal info
    {
        frNum = ((sFrNum % MAX_WIN_SIZE) << 4) | (rFrNum % MAX_WIN_SIZE);
        control = CTL_INFO << 8;
        control = control | pf << 8;
        control = (control | frNum);
    }
    else
    {
        control = ((CTL_SPRV << 8) | (pf << 8)) | (ss | rFrNum);
    }
//    printf("CONTROL HI: %d\n", control >> 8);
//    printf("CONTROL LO: %d\n", control & 0xFF);
    return control;
}

char *getStuffedInfo(char *info)
{
    char stuffedInfo[MAX_INFO_SIZE] = {0};
    int padding = 0;
    for(int i = 0; i < strlen(info); i++)
    {
        stuffedInfo[i+padding] = info[i];
        // found 0x7E. Stuff it
        if(info[i] == (char)0x7E)
        {
            padding++;
            stuffedInfo[i+padding] = 0x7E;
        }
    }
    stuffedInfo[strcspn(stuffedInfo, "\n")] = 0;
    char *retVal = malloc(strlen(stuffedInfo));
    strcpy(retVal, stuffedInfo);
    return retVal;
}

// https://www.geeksforgeeks.org/count-set-bits-in-an-integer/
int countBits(char *string, int start, int end)
{
    int count = 0;
    for(int i = start; i < end; i++)
    {
        unsigned int n = string[i];
        while(n)
        {
            count += n & 1;
            n >>= 1;
        }
    }
    return count;
}

// 2SENTINEL+PREAMBLE+POSTAMBLE = 4bytes
// control                      = 2bytes
// FCS                          = 1byte
// info                         = 1024 - 7 = 1017
char *createFrame(char *info, uint8_t ss, uint8_t pf, int rFrNum, int sFrNum)
{
    updateTime(&timeMSec);
    char *newFrame = malloc(MAX_FRAME_SIZE);
    int index = 0;
    newFrame[index++] = SENTINEL;
    newFrame[index++] = PREAMBLE;
    uint16_t ctl = createControl(CTL_SS_INF, CTL_F, 0, 0);
    newFrame[index++] = ctl >> 8;                       // hi
    newFrame[index++] = ctl & 0xFF;                     // lo
    char *stuffedInfo = getStuffedInfo(info);
    strcpy(newFrame+index, stuffedInfo);
    index += strlen(stuffedInfo)+2;
    if(countBits(newFrame, 0, index-1) % 2 == 0)
    {
        //printf("Even bit count\n");
        newFrame[index++] = 0x00;                           // supposedly file check
    }
    else
    {
        //printf("Odd bit count\n");
        newFrame[index++] = 0x01;                           // supposedly file check
    }
    newFrame[index++] = SENTINEL;
    newFrame[index] = POSTAMBLE;
    return newFrame;
}

void *freeFrameList(FrameList *frameList)
{
    FrameList *frame;
    while(frameList != NULL)
    {
        frame = frameList;
        frameList = frameList->next;
        free(frame);
    }
    return NULL;
}
