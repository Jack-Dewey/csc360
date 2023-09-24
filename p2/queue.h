#ifndef QUEUE_H_
#define QUEUE_H_
#include <stdio.h>
#include <stdlib.h>
#include "customer_info.h"


// From https://www.geeksforgeeks.org/introduction-and-array-implementation-of-queue/
// A structure to represent a queue
typedef struct Queue {
    int front, rear, size;
    unsigned capacity;
    int* array;
    customer_info customer;
}Queue;

struct qNode{
    customer_info customer;
};


// List Function Prototypes
Queue* createQueue(unsigned capacity);
int dequeue(struct Queue* queue);                  // Remove an item from the queue
int isEmpty(struct Queue* queue);                   // Return true if queue is empty
int isFull(struct Queue* queue);                    // Return true if queue is full
int front(struct Queue* queue);
int rear(struct Queue* queue);
void enqueue(struct Queue* queue, int item);        // Enter an item in the queue
// Define TRUE and FALSE if they have not already been defined
#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif

#endif // End of queue header