#ifndef SWITCH_H_
#define SWITCH_H_

#include "table.h"
#include "queue.h"
#include "link.h"

/*
#define NUM_SWITCHES 1 
*/

typedef struct {
	int physId;
	int rootId;
   int parentId;
   int rootDist;
   int in_size;
	int out_size;
	LinkInfo* link_in;
	LinkInfo* link_out;
	Table f_table;
	Queue packet_q;

} SwitchState;

void switchInit(SwitchState* s_state, int physid);
void switchMain(SwitchState* s_state);

#endif
