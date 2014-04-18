#ifndef C_QUEUE_H_
#define C_QUEUE_H_

#include "main.h"

#define MAX_QUEUE_SIZE 10000

typedef packetBuffer Data;

typedef struct {
	Data elements[MAX_QUEUE_SIZE];
	int head;
	int tail;
	int size;
} Queue;

void InitQueue(Queue* queue);
void PushQueue(Queue* queue, Data data);
Data PopQueue(Queue* queue);
void DisplayQueue(Queue* queue);
int  IsEmpty(Queue* queue);

#endif
