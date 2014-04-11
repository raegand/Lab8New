#include <stdio.h>
#include "c_queue.h"
#include "main.h"

int main() {
	Queue test;
	packetBuffer a = {
		.srcaddr = 0,
		.dstaddr = 0,
		.length = 0,
		.valid = 0,
		.start = 0,
		.end = 0
	};
	packetBuffer b = a;
	b.srcaddr = 1;
	packetBuffer c = a;
	c.srcaddr = 2;


	InitQueue(&test);
	PushQueue(&test, a);
	PushQueue(&test, b);
	PushQueue(&test, c);
	DisplayQueue(&test);
	printf("pop queue value is %d\n", PopQueue(&test));
	printf("pop queue value is %d\n", PopQueue(&test));
	printf("pop queue value is %d\n", PopQueue(&test));
	printf("pop queue value is %d\n", PopQueue(&test));
	PushQueue(&test, a);
	PushQueue(&test, b);
	PushQueue(&test, c);
	DisplayQueue(&test);
	printf("popping front of queue\n");
	packetBuffer pop = PopQueue(&test);
	printf("srcaddr of front is %d\n", pop.srcaddr);

	return 0;
}
