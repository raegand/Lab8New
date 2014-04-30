#include <stdio.h>
#include "queue.h"

const packetBuffer ERROR_BUFFER = {
	.srcaddr = -1,
	.dstaddr = -1,
	.length = -1,
	.valid = -1,
	.start = -1,
	.end = -1
};

void InitQueue(Queue* queue) {
	queue->head = 0;
	queue->tail = -1;
	queue->size = 0;
}

void PushQueue(Queue* queue, Data data) {
	if (queue->size >= MAX_QUEUE_SIZE) {
		printf("queue is full\n");
		return;
	}
	queue->tail++;
	queue->tail %= MAX_QUEUE_SIZE;
	queue->elements[queue->tail] = data;
	queue->size++;
}

Data PopQueue(Queue* queue) {
	if (queue->size == 0) {
		printf("queue is empty\n");
		return ERROR_BUFFER;
	}
	queue->head++;
	queue->head %= MAX_QUEUE_SIZE;
	queue->size--;
	return queue->elements[queue->head-1];
}

int IsEmpty(Queue* queue) {
	return queue->size == 0;
}

void DisplayQueue(Queue* queue) {
	int i = 0;
	for (i; i < queue->size; i++) {
//		printf("%d\n", queue->elements[i].srcaddr);
	}
}

