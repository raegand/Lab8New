#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "utilities.h"
#include "link.h"
#include "man.h"
#include "host.h"
#include "databuff.h"
#include "dns.h"

#define TENMILLISEC 10000   /* 10 millisecond sleep */

void dnsInitState(dnsState* dstate, int physid); 
void dnsInitRcvPacketBuff(packetBuffer * packetbuff);
void dnsTransmitPacket(dnsState * dstate, char replymsg[]);
void dnsInitTransmit(dnsState * dstate, char word[], char replymsg[]);

void dnsMain(dnsState * dstate)
{
   char buffer[1000]; /* The message from the manager */
   char word[1000];
   int  value;
   char replymsg[1000];   /* Reply message to be displayed at the manager */
   packetBuffer tmpbuff;
   int length = 0; /* Size of string in pipe */

   while(1) {

      /* Check if there is an incoming packet */
      length = linkReceive(&(dstate->linkin), &tmpbuff);

      if (tmpbuff.dstaddr == dstate->netaddr && tmpbuff.valid == 1 && tmpbuff.type == 2) {
         /* Do stuff with NTABLE + CHECK VALIDTY THEN RESPOND*/
     
     
     }
     
      /* The host goes to sleep for 10 ms */
      usleep(TENMILLISEC);
   } /* End of while loop */
}

void dnsInit(dnsState* dstate, int physid)
{
   dnsInitState(dstate, physid);    
   dnsInitRcvPacketBuff(&(dstate->rcvPacketBuff));  
   InitNTable(&(dstate->n_table));
}

void dnsInitRcvPacketBuff(packetBuffer * packetbuff)
{
   packetbuff->valid = 0;
   packetbuff->end = 0;
   packetbuff->start = 0;
}

void dnsInitState(dnsState * dnsstate, int physid)
{
   dnsstate->physid = physid;
   dnsstate->netaddr = physid; /* default address */  
   dnsstate->rcvPacketBuff.valid = 0;
}

