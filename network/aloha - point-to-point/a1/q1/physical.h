#ifndef _PHYSICAL_H
#define _PHYSICAL_H

#include <pthread.h>

#define MAX_FRAME_SIZE 1024

typedef enum
{
  false,
  true
} boolean;

// defines the to-from direction of a transmission
// The "left" peer transmits on L2R and receives on R2L
// The "right" peer transmits on R2L and receives on L2R
typedef enum
{
  L2R,
  R2L
} Direction;

// Your code must wait on condition variable signals from the physical layer 
// before transmitting or starting a receive.

// After initialization the physical layer is ready to transmit (see booleans below).
// Following pthread code structuring (see 3430), you shouldn't enter a wait if the state indicates ready.
// Otherwise you'll be waiting for a signal that will never come.

// Your code is responsible for properly locking the physical layer -- 
// if I did it we'd end up in deadlock on the condition vars.
struct PHYSICAL_STATE
{
  pthread_cond_t  L2RTxSignal;     // wait on this before transmitting on L2R
  pthread_cond_t  L2RRxSignal;     // wait on this before receiving from L2R
  boolean         L2RReady;        // true == ready to transmit, false == bytes to receive
  pthread_mutex_t L2RLock;         // make sure you use the lock when working with L2R

  pthread_cond_t  R2LTxSignal;     // wait on this before transmitting on R2L
  pthread_cond_t  R2LRxSignal;     // wait on this before receiving from R2L
  boolean         R2LReady;        // true == ready to transmit, false == bytes to receive
  pthread_mutex_t R2LLock;         // make sure you use the lock when working with R2L
};

typedef struct PHYSICAL_STATE PhysicalState;

// call before doing anything with the physical layer
PhysicalState * initPhysical();
// call when you're done with the physical layer
void cleanPhysical();

unsigned char receiveByte(const Direction dir);
void transmitFrame(const Direction dir, unsigned char const * const frame, const int length);

#endif

