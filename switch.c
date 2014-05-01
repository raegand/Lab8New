#include <stdio.h>
#include <sys/types.h>

#include "switch.h"
#include "link.h"
#include "queue.h"

/* Is actually arbitrary value for Neighbor */
#define INF  99999
#define NEIGHBOR 99999
#define TEN_MILLI_SEC 10000
#define PIPE_WRITE 1
#define PIPE_READ 0
#define DATA 0
#define INFO 1
#define CHAR 1
#define ISCHILD 1
#define HUNDRED_MILLI_SEC 10


void writeSwitchData(SwitchState * s_state) 
{
   /* Writes switch data to file for debug purposes */
   FILE * debug = fopen("DEBUG_SWITCH", "a");
   fprintf(debug, "Switch ID: %d ", s_state->physId);
   fprintf(debug, "| Root: %d | ", s_state->rootId);
   fprintf(debug, "Dist: %d | ", s_state->rootDist);
   fprintf(debug, "ParentId: %d \n", s_state->parentId);
   fclose(debug); 
   SwitchDebugTable(&(s_state->f_table), s_state->physId);
}

void switchInit(SwitchState* s_state, int physid) {
	s_state->in_size = 0;
	s_state->out_size = 0;
	s_state->physId = physid;
   s_state->rootId = physid; /* initially rootid is physid */
   s_state->parentId = physid; /* initially own parent */
   s_state->rootDist = INF; /* initially infinite distance from root */
	InitQueue(&(s_state->packet_q));
	InitTable(&(s_state->f_table));
}

void transmitAll(SwitchState* s_state, packetBuffer* pb)
{
   /* this function is specialized for INFO packets */
   int i;
   for(i = 0; i < s_state->out_size; i++) {
      int x = s_state->link_out[i].uniPipeInfo.physIdDst;
      if(x == s_state->parentId) {
         pb->parent = 1;
      } else {
         pb->parent = 0;
      }
      linkSend(&(s_state->link_out[i]), pb);
   }
}

void transmitGoodLink(SwitchState* s_state, packetBuffer* pb, int in)
{
   int i;
   for(i = 0; i < s_state->out_size; i++) {
      if(i!= in) {
         int bad = IsBad(&(s_state->f_table), 
         s_state->link_out[i].uniPipeInfo.physIdDst);
         if(bad == 0) {
            linkSend(&(s_state->link_out[i]), pb);
         }
      }
   }
}

void transmitRoot(SwitchState* s_state)
{
   packetBuffer temp;
   temp.type = INFO;
   temp.srcaddr = s_state->physId;
   temp.dstaddr = NEIGHBOR;
   temp.length = CHAR;
   temp.valid = 1;
   temp.distance = s_state->rootDist;
   temp.root = s_state->rootId; 
   transmitAll(s_state, &temp);
}

void UpdateRoot(SwitchState* s_state, packetBuffer* pb) 
{
   /*
    * ---------------------- PACKET DATA: ----------------------
    * Root Id | pb->root
    * Distance | pb->Dist
    * Parent Flag | pb->parent (if == 1, then it was sent by child)
    * ----------------------------------------------------------
    */

   /* Behavioral fixes*/
   if(s_state->rootDist == 0 && s_state->physId != s_state->rootId) {
      s_state->rootDist = INF;
   }

   /* If neighbors rootid is ME, I am root, therefor dist = 0 */
   if(pb->root == s_state->physId) {
      s_state->rootDist = 0;
      UpdateChildData(&(s_state->f_table), pb->srcaddr);
      return;
   }
  
   /* If root in packet is smaller than me, update my own root */ 
   if(pb->root < s_state->rootId) {
      s_state->rootId = pb->root;
      s_state->rootDist = INF; // Don't know distance yet
      return;
   }

   /* If neighbor doesn't have same root as me, then ignore/exit */
   if(pb->root != s_state->rootId) return;
  
   if(pb->srcaddr == s_state->rootId) {
      s_state->parentId = pb->srcaddr;
   }
   
   if(pb->distance < s_state->rootDist) {
      /* Changing parent won't matter if this case */
      if(s_state->rootDist == pb->distance + 1) {
         if(pb->srcaddr != s_state->parentId) {
            UpdateBad(&(s_state->f_table), pb->srcaddr);
         }
         return;
      }
      /* Update Distance */
      s_state->rootDist = pb->distance + 1;
      s_state->parentId = pb->srcaddr;
      UpdateParentData(&(s_state->f_table), pb->srcaddr);
   } else if (pb->distance == s_state->rootDist) {
      UpdateBad(&(s_state->f_table), pb->srcaddr);
   } else if(pb->distance > s_state->rootDist) {
      if(pb->parent == 0) {
         UpdateBad(&(s_state->f_table), pb->srcaddr);
      } else {
         UpdateChildData(&(s_state->f_table), pb->srcaddr);
      }
   }

   if(pb->parent == 1) {
         UpdateChildData(&(s_state->f_table), pb->srcaddr);
   }


}

void switchMain(SwitchState* s_state) {
	int i = 0;
	int packet_size = 0;
	int out_link = 0;
	int in_link = 0;
	packetBuffer tmpbuff;
   int timer = 0;
	while(1) {
		/* go through all incoming links and check if there is something
		 * to be received */
		for (i = 0; i < s_state->in_size; i++) {
			packet_size = linkReceive(&(s_state->link_in[i]), 
									  &tmpbuff);	
			/* if there is a packet */
			if (packet_size > 0) {
            if(tmpbuff.type != INFO) {
               /* either add value to table, or update existing value 
                * NOTE - we are using srcaddr to put into table */
               UpdateTable(&(s_state->f_table), tmpbuff.valid, 
                        tmpbuff.srcaddr, i);
               PushQueue(&(s_state->packet_q), tmpbuff);
            } else {
               UpdateTable(&(s_state->f_table), tmpbuff.valid, 
                        tmpbuff.srcaddr, i);
               UpdateRoot(s_state, &tmpbuff); 
            }
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
               transmitGoodLink(s_state, &tmpbuff, in_link);
				}
			}
      } else {
         //periodical transmit roots, only when packetqueue is empty
         if(timer == HUNDRED_MILLI_SEC) {
            transmitRoot(s_state);
            timer = 0;
         }
      }
      /* DEBUG PURPOSE ONLY */
      writeSwitchData(s_state);      
		usleep(TEN_MILLI_SEC);
      timer++;
	}

}
