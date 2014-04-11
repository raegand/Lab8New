#ifndef QUEUE_H_
#define QUEUE_H_

#include "main.h"
typedef packetBuffer Data;

typedef struct Node {
	Data data;
	struct Node* next;
} Node;

typedef struct {
	Node* head;
	Node* tail;
	int   size;
} Queue;

void InitQueue(Queue* queue);
void PushQueue(Queue* queue, Data data);
Data PopQueue(Queue* queue);
int  IsEmpty(Queue* queue);
void DisplayQueue(Queue* queue);

#endif

