#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	queue->head = NULL;
	queue->tail = NULL; 
	queue->size = 0;
}

int IsEmpty(Queue* queue) {
	return queue->size == 0;
}

void PushQueue(Queue* queue, Data data) {
	Node* newNode = (Node*)malloc(sizeof(Node));
	if (newNode == NULL) {
		printf("couldn't add value to queue");
		return;
	}
	newNode->next = NULL;
	newNode->data = data;
	if (queue->size == 0) {
		queue->head = newNode;
		queue->tail = newNode;
	} else {
		queue->tail->next = newNode;
		queue->tail = queue->tail->next;
	}
	queue->size++;
}

Data PopQueue(Queue* list) {
	if (list->size == 0) {
		printf("empty queue\n");
		return ERROR_BUFFER;
	}

	Node* delNode = list->head;
	Data data = delNode->data;
	list->head = list->head->next;
	if (list->size == 1) {
		list->tail = NULL;
	}
	free(delNode);
	list->size--;
	return data;
}

void DisplayQueue(Queue* queue) {
	Node* node = queue->head;
	for (node; node != NULL; node = node->next) {
		printf("%d\n", node->data.srcaddr);
	}
}



