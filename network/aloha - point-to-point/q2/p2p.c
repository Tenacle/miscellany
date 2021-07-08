#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include "physical.h"
//#include <math.h>

pthread_mutex_t mutex;
pthread_cond_t condA, condB;

int buffer[100];

enum {STATE_A, STATE_B} state = STATE_A;
int loops = 5;
int length = 0;
boolean s = STATE_A;

void *producer(void *arg) {
    int i;
    //printf("b: %c\n", b);
    for (i = 0; i < loops; i++) {
        pthread_mutex_lock(&mutex);
        while(s != STATE_A) {
            printf("************* producer waiting...\n");
            pthread_cond_wait(&condA, &mutex);
        }
        printf("RUNNING PRODUCER A\n");
        pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutex);
        buffer[length++] = i;
        printf("Producer producing item %d (curr item size: %d)\n", i, length);
        s = STATE_B;
        printf("sending signal to consumer B\n");
        pthread_cond_signal(&condB);
        pthread_mutex_unlock(&mutex);
    }
}

void *consumer(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
        pthread_mutex_lock(&mutex);
        while(s != STATE_B) {
            printf("************* consumer waiting...\n");
            pthread_cond_wait(&condB, &mutex);
        }
        printf("RUNNING CONSUMER B\n");
        pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutex);
        int item = buffer[--length];
        printf("Consumer consuming item %d (items left: %d)\n", item, length);
        s = STATE_A;
        printf("sending signal to producer A\n");
        pthread_cond_signal(&condA);
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char *argv[])
{

    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&condA, 0);
    pthread_cond_init(&condB, 0);

    pthread_t pThread, cThread;
    pthread_create(&pThread, 0, producer, 0);
    pthread_create(&cThread, 0, consumer, 0);
    pthread_join(pThread, NULL);
    pthread_join(cThread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condA);
    pthread_cond_destroy(&condB);
    return 0;
}

