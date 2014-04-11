#include <stdio.h>
#include <sys/types.h>

#include "switch.h"
#include "link.h"
#include "queue.h"

#define TEN_MILLI_SEC 10000
#define PIPE_WRITE 1
#define PIPE_READ 0

void switchInit(SwitchState* s_state, int physid) {
	s_state->in_size = 0;
	s_state->out_size = 0;
	s_state->physId = physid;
	InitQueue(&(s_state->packet_q));
	InitTable(&(s_state->f_table));
}

void switchMain(SwitchState* s_state) {
	int i = 0;
	int packet_size = 0;
	int out_link = 0;
	int in_link = 0;
	packetBuffer tmpbuff;
	while(1) {
		/* go through all incoming links and check if there is something
		 * to be received */
		for (i = 0; i < s_state->in_size; i++) {
			packet_size = linkReceive(&(s_state->link_in[i]), 
									  &tmpbuff);	
			/* if there is a packet */
			if (packet_size > 0) {
				/* either add value to table, or update existing value 
				 * NOTE - we are using srcaddr to put into table */
				UpdateTable(&(s_state->f_table), tmpbuff.valid, 
							tmpbuff.srcaddr, i);
				PushQueue(&(s_state->packet_q), tmpbuff);
			}
		}
		/* if queue is not empty */
		if (!(IsEmpty(&(s_state->packet_q)))) {
			tmpbuff = PopQueue(&(s_state->packet_q));
			if (tmpbuff.srcaddr != -1) {
				out_link = GetOutLink(&(s_state->f_table), tmpbuff.dstaddr);
				/* if inside table */
				if (out_link != ERROR) {
					linkSend(&(s_state->link_out[out_link]), &tmpbuff);
				} else {
					/* we know we that we have the srcaddr link b/c we input it
					 * when we receive a packet */
					in_link = GetOutLink(&(s_state->f_table), tmpbuff.srcaddr);
					/* else send it to all but incoming link */
					for (i = 0; i < s_state->out_size; i++) {
						if (i != in_link) {
							linkSend(&(s_state->link_out[i]), &tmpbuff);
						}
					}
				}
			}
		}

		usleep(TEN_MILLI_SEC);
	}

}
