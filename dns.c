/* 
 * This is the source code for the host.  
 * hostMain is the main function for the host.  It is an infinite
 * loop that repeatedy polls the connection from the manager and
 * its input link.  
 *
 * If there is command message from the manager,
 * it parses the message and executes the command.  This will
 * result in it sending a reply back to the manager.  
 *
 * If there is a packet on its incoming lik, it checks if
 * the packet is destined for it.  Then it stores the packet
 * in its receive packet buffer.
 *
 * There is also a 10 millisecond delay in the loop caused by
 * the system call "usleep".  This puts the host to sleep.  This
 * should reduce wasted CPU cycles.  It should also help keep
 * all nodes in the network to be working at the same rate, which
 * helps ensure no node gets too much work to do compared to others.
 */

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

#define EMPTY_ADDR  0xffff  /* Indicates that the empty address */
                             /* It also indicates that the broadcast address */
#define MAXBUFFER 3000
#define PIPEWRITE 1 
#define PIPEREAD  0
#define TENMILLISEC 10000   /* 10 millisecond sleep */

void dnsInitState(dnsState* dstate, int physid); 
void dnsInitRcvPacketBuff(packetBuffer * packetbuff);
void dnsInitSendPacketBuff(packetBuffer * packetbuff);
int  dnsCommandReceive(managerLink * manLink, char command[]);
void dnsSetNetAddr(dnsState * dstate, int netaddr, char replymsg[]);
void dnsSetMainDir(dnsState * dstate, char filename[], char replymsg[]);
void dnsClearRcvFlg(dnsState * dstate, char replymsg[]);
void dnsUploadPacket(dnsState * dstate, char fname[], char replymsg[]); 
void dnsDownloadPacket(dnsState * dstate, char fname[], char replymsg[]); 
void dnsTransmitPacket(dnsState * dstate, char replymsg[]);
void dnsGetHostState(dnsState * dstate, managerLink * manLink, char replymsg[]);
void dnsReplySend(managerLink * manLink, char replytype[], char reply[]);
void dnsToManSend(managerLink * manLink, char reply[]);
void dnsInitDataBuffer(DataBuffer * buff);
void dnsInitTransmit(dnsState * dstate, char word[], char replymsg[]);

/*
 * hostTransmitPacket will transmit a packet in the send packet buffer
 */
void dnsTransmitPacket(dnsState * dstate, char replymsg[])
{
	int i = 0;
	int length = dstate->sendBuffer.length - dstate->sendBuffer.pos;
	int error = 0;
	if (length > PAYLOAD_LENGTH) {
		length = PAYLOAD_LENGTH;	
	}
	
   dstate->sendPacketBuff.dstaddr = dstate->sendBuffer.dstaddr;
	dstate->sendPacketBuff.srcaddr = dstate->netaddr;
	dstate->sendPacketBuff.length = length;
	dstate->sendPacketBuff.type = 3;
   dstate->sendPacketBuff.end = 0;
	dstate->sendPacketBuff.start = 0;
	dstate->sendPacketBuff.root = 0;
   dstate->sendPacketBuff.distance = 0;
   
   if (dstate->sendBuffer.pos == 0) {
		dstate->sendPacketBuff.start = 1;
	}
	if (dstate->sendBuffer.valid) {
		dstate->sendPacketBuff.valid = 1;
	}

	for (i = 0; i < length; i++) {
		dstate->sendPacketBuff.payload[i] = 
			dstate->sendBuffer.data[i+dstate->sendBuffer.pos];
	}

	dstate->sendBuffer.pos += length;

	if (dstate->sendBuffer.pos >= dstate->sendBuffer.length) {
		dstate->sendBuffer.pos = 0;
		dstate->sendBuffer.busy = 0;
		dstate->sendPacketBuff.end = 1;
	}
	
	/* Transmit the packet on the link */
	error = linkSend(&(dstate->linkout), &(dstate->sendPacketBuff));
	if (error == -1) {
		strcpy(replymsg,"Error: Could not send packet, aborting transmit");
		hostInitDataBuffer(&(dstate->sendBuffer));
		return;
	}

	if (dstate->sendPacketBuff.end == 1) {
		strcpy(replymsg,"Final Packet Sent");
	} else {
		/* Message to be sent back to the manager */
		strcpy(replymsg,"Packet sent");
	}
}

void dnsInitTransmit(dnsState * dstate, char word[], char replymsg[]) {
	if (dstate->sendBuffer.busy == 1) {
   		strcpy(replymsg, "Transmit Aborted: Currently Trasmitting");
		return;
	}
	if (dstate->sendBuffer.valid == 0) {
   		strcpy(replymsg, "Transmit Aborted: No File to Transmit");
		return;
	}

	char dest[1000];
	int  dstaddr;
	
	findWord(dest, word, 2);
	dstaddr = ascii2Int(dest);
	dstate->sendBuffer.dstaddr = dstaddr;
	dstate->sendBuffer.busy = 1;
	dstate->sendBuffer.pos = 0;
   strcpy(replymsg, "Transmit Started");
}

void dnsMain(dnsState * dstate)
{
   char buffer[1000]; /* The message from the manager */
   char word[1000];
   int  value;
   char replymsg[1000];   /* Reply message to be displayed at the manager */
   packetBuffer tmpbuff;
   int length = 0; /* Size of string in pipe */

   while(1) {
      /* Check if there is a command message from the manager */
      length = hostCommandReceive(&(dstate->manLink),buffer);

      if (dstate->sendBuffer.busy == 1) {
         hostTransmitPacket(dstate, replymsg);
      }

      /* Check if there is an incoming packet */
      length = linkReceive(&(dstate->linkin), &tmpbuff);

      if (tmpbuff.dstaddr == dstate->netaddr && tmpbuff.valid == 1 && tmpbuff.type == 3) {
           /* if there is already something in the buffer; clear it */
           if (tmpbuff.start == 1) {
              memset(dstate->rcvBuffer.data, 0, sizeof(dstate->rcvBuffer.data));
              dstate->rcvBuffer.length = 0;
              dstate->rcvflag = 0;
     }
     
     strcat(dstate->rcvBuffer.data, tmpbuff.payload);
     dstate->rcvBuffer.length += tmpbuff.length;
     dstate->rcvBuffer.srcaddr = tmpbuff.srcaddr;
     dstate->rcvBuffer.dstaddr = tmpbuff.dstaddr;
     if (tmpbuff.end == 1) {
        dstate->rcvflag = 1;
        dstate->rcvBuffer.valid = 1;
     }
   }
      /* The host goes to sleep for 10 ms */
      usleep(TENMILLISEC);
   } /* End of while loop */
}

int dnsCommandReceive(managerLink * manLink, char command[])
{
   int n;
   n = read(manLink->toHost[PIPEREAD],command,250);
   command[n] = '\0';
   return n+1;
}

void dnsToManSend(managerLink * manLink, char reply[])
{
   write(manLink->fromHost[PIPEWRITE],reply,strlen(reply));
}

void dnsReplySend(managerLink * manLink, char replytype[], char replymsg[])
{
   char reply[1000];

   reply[0] = '\0';
   appendWithSpace(reply, replytype);
   appendWithSpace(reply, replymsg);
   dnsToManSend(manLink, reply);
}

void dnsInit(dnsState* dstate, int physid)
{
   dnsInitState(dstate, physid);     /* Initialize host's state */
   /* Initialize the receive and send packet buffers */
   dnsInitRcvPacketBuff(&(dstate->rcvPacketBuff));  
   dnsInitSendPacketBuff(&(dstate->rcvPacketBuff)); 

void dnsInitSendPacketBuff(packetBuffer * packetbuff)
{
   packetbuff->valid = 0;
   packetbuff->end = 0;
   packetbuff->start = 0;
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
   dnssate->rcvflag = 0;
   dnsInitDataBuffer(&(dstate->sendBuffer));
   dnsInitDataBuffer(&(dstate->rcvBuffer));
}

