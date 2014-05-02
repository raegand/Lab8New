/* 
 * This is the source code for the host.  
 * hostMain is the main function for the host.  It is an infinite
 * loop that repeatedy polls the connection from the manager and
/bin/bash: 6: command not found
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

void hostUploadAndTransmit(hostState * hstate, char fname[], int src);
void hostRequestFile(hostState * hstate, char filename[], int addr); 
void hostSilentTransmitPacket(hostState * hstate); 
void hostSilentInitTransmit(hostState * hstate, int src); 
void writeHostData(hostState * hstate) 
{
   /* Writes switch data to file for debug purposes */
   FILE * debug = fopen("DEBUG_HOST", "a");
   fprintf(debug, "Host ID: %d ", hstate->physid);
   int out, out2, in, in2;
   out = (hstate->linkout).uniPipeInfo.physIdDst;
   out2= (hstate->linkout).uniPipeInfo.physIdSrc;
   in = (hstate->linkin).uniPipeInfo.physIdDst;
   in2= (hstate->linkin).uniPipeInfo.physIdSrc;

   fprintf(debug, "- OutDst/Src: %d,%d - ", out, out2);
   fprintf(debug, "InDst/Src: %d %d\n", in, in2);
   fclose(debug);
}

/* 
 * hostInit initializes the host.  It calls
 * - hostInitState which initializes the host's state.
 * - hostInitRcvPacketBuff, which initializes the receive packet buffer
 * - hostInitSendPacketBuff, which initializes the send packet buffer
 */
void hostInitState(hostState * hstate, int physid); 
void hostInitRcvPacketBuff(packetBuffer * packetbuff);
void hostInitSendPacketBuff(packetBuffer * packetbuff);

/*
 * hostMain is the main loop for the host. It has an infinite loop.
 * In the loop it first calls
 * hostCommandReceive to check if a command message came from the
 * manager.
 * If a command arrived, then it checks the first word of the
 * message to determine the type of command.  Depending on the
 * command it will call
 * - hostSetNetAddr to set the host's network address
 *      The command message should be "SetNetAddr <network address>"
 * - hostSetMainDir to set the host's main directory
 *      The command message should be "SetMainDir <directory name>"
 * - hostClearRcvFlg to clear the host's receive flag
 * - hostUploadPacket to upload a file to the host's send packet
 *      buffer. The command message should be "UploadPacket <file name>"
 * - hostDownloadPacket to download the payload of the host's
 *      receive packet buffer to a file.  The command message
 *      should be "DownloadPacket <file name>"
 * - hostTransmitPacket to transmit the packet in the send packet buffer.
 *      The command message should be "TransmitPacket <destination address>"
 * - hostGetHostState to get the host's state.  The command message
 *      should be "GetHostState".  
 */
int  hostCommandReceive(managerLink * manLink, char command[]);

void hostReqAddr(hostState * hstate, char hname[], char replymsg[]);
void hostSetNetAddr(hostState * hstate, int netaddr, char replymsg[]);
void hostSetMainDir(hostState * hstate, char filename[], char replymsg[]);
void hostSetName(hostState * hstate, char hname[], char replymsg[]);
void hostClearRcvFlg(hostState * hstate, char replymsg[]);
void hostUploadPacket(hostState * hstate, char fname[], char replymsg[]); 
void hostDownloadPacket(hostState * hstate, char fname[], char replymsg[]); 
void hostTransmitPacket(hostState * hstate, char replymsg[]);
void hostGetHostState(hostState * hstate, managerLink * manLink, char replymsg[]);

/* 
 * After host executes a command from the manager, it sends a reply
 * message to the manager in the format " Replytype <reply[]>".
 * - Replytype can be
 *    o "DISPLAY" which means the rest of the message should be 
 *      displayed on the user's console
 *    o "GetHostStateACK" which means the rest of the message is the
 *      host's state
 */ 
void hostReplySend(managerLink * manLink, char replytype[], char reply[]);

/* This is used to send a message to the manager */
void hostToManSend(managerLink * manLink, char reply[]);

void hostInitDataBuffer(DataBuffer * buff);
void hostInitTransmit(hostState * hstate, char word[], char replymsg[]);

/*
 * Functions
 */

/*
 * hostTransmitPacket will transmit a packet in the send packet buffer
 */
void hostTransmitPacket(hostState * hstate, char replymsg[])
{
	int i = 0;
	int length = hstate->sendBuffer.length - hstate->sendBuffer.pos;
	int error = 0;
	if (length > PAYLOAD_LENGTH) {
		length = PAYLOAD_LENGTH;	
	}
	/* 
	 * Set up the send packet buffer's source and destination addresses
	 */
	hstate->sendPacketBuff.dstaddr = hstate->sendBuffer.dstaddr;
	hstate->sendPacketBuff.srcaddr = hstate->netaddr;
	hstate->sendPacketBuff.length = length;
	hstate->sendPacketBuff.type = 0;
   hstate->sendPacketBuff.end = 0;
	hstate->sendPacketBuff.start = 0;
	hstate->sendPacketBuff.root = 0;
   hstate->sendPacketBuff.distance = 0;
   
   if (hstate->sendBuffer.pos == 0) {
		hstate->sendPacketBuff.start = 1;
	}
	if (hstate->sendBuffer.valid) {
		hstate->sendPacketBuff.valid = 1;
	}

	for (i = 0; i < length; i++) {
		hstate->sendPacketBuff.payload[i] = 
			hstate->sendBuffer.data[i+hstate->sendBuffer.pos];
	}

	hstate->sendBuffer.pos += length;

	if (hstate->sendBuffer.pos >= hstate->sendBuffer.length) {
		hstate->sendBuffer.pos = 0;
		hstate->sendBuffer.busy = 0;
		hstate->sendPacketBuff.end = 1;
	}
	
	/* Transmit the packet on the link */
	error = linkSend(&(hstate->linkout), &(hstate->sendPacketBuff));
	if (error == -1) {
		strcpy(replymsg,"Error: Could not send packet, aborting transmit");
		hostInitDataBuffer(&(hstate->sendBuffer));
		return;
	}

	if (hstate->sendPacketBuff.end == 1) {
		strcpy(replymsg,"Final Packet Sent");
	} else {
		/* Message to be sent back to the manager */
		strcpy(replymsg,"Packet sent");
	}
}

void hostInitTransmit(hostState * hstate, char word[], char replymsg[]) {
	if (hstate->sendBuffer.busy == 1) {
   		strcpy(replymsg, "Transmit Aborted: Currently Trasmitting");
		return;
	}
	if (hstate->sendBuffer.valid == 0) {
   		strcpy(replymsg, "Transmit Aborted: No File to Transmit");
		return;
	}

	char dest[1000];
	int  dstaddr;
	
	findWord(dest, word, 2);
	dstaddr = ascii2Int(dest);
	hstate->sendBuffer.dstaddr = dstaddr;
	hstate->sendBuffer.busy = 1;
	hstate->sendBuffer.pos = 0;
   	strcpy(replymsg, "Transmit Started");
}


/* 
 * Main loop of the host node
 *
 * It polls the manager connection for any requests from
 * the manager, and replies
 *
 * Then it polls any incoming links and downloads any
 * incoming packets to its receive packet buffer
 *
 * Then it sleeps for 10 milliseconds
 *
 * Then back to the top of the loop
 *
 */
void hostMain(hostState * hstate)
{
char buffer[1000]; /* The message from the manager */
char word[1000];
int  value;
char replymsg[1000];   /* Reply message to be displayed at the manager */
packetBuffer tmpbuff;
int length = 0; /* Size of string in pipe */

while(1) {
   /* Check if there is a command message from the manager */
   length = hostCommandReceive(&(hstate->manLink),buffer);

   if (length > 1) { /* Execute the manager's command */
      findWord(word, buffer, 1);
      if (strcmp(word, "SetNetAddr")==0) {
         findWord(word, buffer, 2); /* Find net address */
         value = ascii2Int(word);   /* Convert it to integer */
         hostSetNetAddr(hstate, value, replymsg);
         hostReplySend(&(hstate->manLink),"DISPLAY",replymsg);
      }
      else if (strcmp(word, "SetMainDir")==0) {
         findWord(word, buffer, 2); /* Find directory name */
         hostSetMainDir(hstate, word, replymsg);
         hostReplySend(&(hstate->manLink),"DISPLAY",replymsg);
      }
      else if (strcmp(word, "ReqHostAddr")==0) {
         findWord(word, buffer, 2); /* Find directory name */
         hostReqAddr(hstate, word, replymsg);
      }
      else if (strcmp(word, "RegHostName")==0) {
         findWord(word, buffer, 2); /* Find directory name */
         hostSetName(hstate, word, replymsg);
      }
      else if (strcmp(word, "ClearRcvFlg")==0) {
         hostClearRcvFlg(hstate, replymsg);
         hostReplySend(&(hstate->manLink),"DISPLAY",replymsg);
      }
      else if (strcmp(word, "UploadPacket")==0) {
         findWord(word, buffer, 2); /* Find file name */
         hostUploadPacket(hstate, word, replymsg); 
         hostReplySend(&(hstate->manLink), "DISPLAY",replymsg);
      }
      else if (strcmp(word, "DownloadPacket")==0) {
         findWord(word, buffer, 2); /* Find file name */
         hostDownloadPacket(hstate, word, replymsg);
         hostReplySend(&(hstate->manLink), "DISPLAY",replymsg);
      }
      else if (strcmp(word, "GetHostState")==0) {
         hostGetHostState(hstate, &(hstate->manLink), replymsg);
         hostReplySend(&(hstate->manLink), "GetHostStateAck",replymsg);
      }
      else if (strcmp(word, "TransmitPacket")==0) {
		 hostInitTransmit(hstate, buffer, replymsg);
		 hostReplySend(&(hstate->manLink), "DISPLAY",replymsg);
      }
      else if (strcmp(word, "RequestFile")==0) {
       char filename[100];
       char dest[10];
       findWord(filename, buffer, 2); /* file name is apeended before addr*/
       findWord(dest, buffer, 3); /* Address is appended after filename */
       int addr = ascii2Int(dest); 
       hostRequestFile(hstate, filename, addr);
      }
   } /* end of if */

   if (hstate->sendBuffer.busy == 1) {
	   hostTransmitPacket(hstate, replymsg);
   }

   /* Check if there is an incoming packet */
   length = linkReceive(&(hstate->linkin), &tmpbuff);

   /* 
    * If there is a packet and if the packet's destination address 
    * is the host's network address then store the packet in the
    * receive packet buffer
    */
   if (tmpbuff.dstaddr == hstate->netaddr && tmpbuff.valid == 1 && tmpbuff.type == 0) {
        /* if there is already something in the buffer; clear it */
        if (tmpbuff.start == 1) {
           memset(hstate->rcvBuffer.data, 0, sizeof(hstate->rcvBuffer.data));
           hstate->rcvBuffer.length = 0;
           hstate->rcvflag = 0;
	  } 
     
     strcat(hstate->rcvBuffer.data, tmpbuff.payload);
	  hstate->rcvBuffer.length += tmpbuff.length;
	  hstate->rcvBuffer.srcaddr = tmpbuff.srcaddr;
	  hstate->rcvBuffer.dstaddr = tmpbuff.dstaddr;
	  if (tmpbuff.end == 1) {
		  hstate->rcvflag = 1;
		  hstate->rcvBuffer.valid = 1;
	  }
   } else if (tmpbuff.dstaddr == hstate->netaddr && tmpbuff.valid == 1 && tmpbuff.type == 6) {
      hostUploadAndTransmit(hstate, tmpbuff.payload, tmpbuff.srcaddr);
   }

   writeHostData(hstate); 

   /* The host goes to sleep for 10 ms */
   usleep(TENMILLISEC);

} /* End of while loop */

}




/*
 * Sets the host's network address.  Also creates a reply message
 * to the manager.
 */
void hostSetNetAddr(hostState * hstate, int netaddr, char replymsg[])
{
hstate->netaddr = netaddr;
strcpy(replymsg, "Network address is set");
}

/* 
 * The host checks the connection from the manager.  If the manager sent
 * a command, the host stores it in "command[]".  It returns the length
 * of the received message + 1.
 */ 

int hostCommandReceive(managerLink * manLink, char command[])
{
int n;
n = read(manLink->toHost[PIPEREAD],command,250);
command[n] = '\0';
return n+1;
}

/* 
 * The host sends a message to the manager.
 */
void hostToManSend(managerLink * manLink, char reply[])
{
write(manLink->fromHost[PIPEWRITE],reply,strlen(reply));
}

/* 
 * hostReplySend is called after the host executes a command 
 * from the manager. It sends a reply
 * message to the manager in the format " Replytype <reply[]>".
 * - Replytype can be
 *    o "DISPLAY" which means the rest of the message should be 
 *      displayed on the user's console
 *    o "GetHostStateACK" which means the rest of the message is the
 *      host's state
 */ 
void hostReplySend(managerLink * manLink, char replytype[], char replymsg[])
{
char reply[1000];

reply[0] = '\0';
appendWithSpace(reply, replytype);
appendWithSpace(reply, replymsg);
hostToManSend(manLink, reply);
}

/* 
 * Initializes the host.   
 */
void hostInit(hostState * hstate, int physid)
{

hostInitState(hstate, physid);     /* Initialize host's state */

/* Initialize the receive and send packet buffers */
hostInitRcvPacketBuff(&(hstate->rcvPacketBuff));  
hostInitSendPacketBuff(&(hstate->rcvPacketBuff)); 
}

/* 
 * Initialize send packet buffer 
 */
void hostInitSendPacketBuff(packetBuffer * packetbuff)
{
packetbuff->valid = 0;
packetbuff->end = 0;
packetbuff->start = 0;
}


/* 
 * Upload a file in the main directory into the send packet buffer
 */
void hostUploadPacket(hostState * hstate, char fname[], char replymsg[]) 
{
	char c;
	FILE * fp;
	char path[MAXBUFFER];  /* Path to the file */
	char tempbuff[MAXBUFFER]; /* A temporary buffer */
	int length;
	int i;

	/* 
	 * Upload the file into tempbuff 
	 */

	if (hstate->maindirvalid == 0) {
		strcpy(replymsg, "Upload aborted: the host's main directory is not yet chosen");
		return;
	}

	/* Create a path to the file and then open it */
	strcpy(path,"");
	strcat(path, hstate->maindir);
	strcat(path, "/");
	strcat(path, fname);

	fp = fopen(path,"rb"); 
	if (fp == NULL) { /* file didn't open */
		strcpy(replymsg, "Upload aborted: the file didn't open");
		return;
	}

	length = fread(tempbuff, 1, MAX_DATA_LENGTH+1, fp);

	if (length==0) {
		strcpy(replymsg, "Upload aborted: error in reading the file");
		return;
	}
	else if (length > MAX_DATA_LENGTH) {
		strcpy(replymsg, "Upload aborted: file is too big");
		return;
	}

	tempbuff[length] = '\0';

	/* Fill in send packet buffer */
	hstate->sendBuffer.valid = 1;
	hstate->sendBuffer.length = length;

	memset(hstate->sendBuffer.data, 0, sizeof(hstate->sendBuffer.data));
	for (i=0; i<length; i++) { /* Store tempbuff in payload of packet buffer */
		hstate->sendBuffer.data[i] = tempbuff[i];
	}

	/* Message to the manager */
	strcpy(replymsg, "Upload successful");

	fclose(fp);
}

void hostUploadAndTransmit(hostState * hstate, char fname[], int src)
{
	char c;
	FILE * fp;
	char path[MAXBUFFER];  /* Path to the file */
	char tempbuff[MAXBUFFER]; /* A temporary buffer */
	int length;
	int i;

	strcat(path, fname);
	fp = fopen(path,"rb"); 
	if (fp == NULL) { /* file didn't open */
		return;
	}

	length = fread(tempbuff, 1, MAX_DATA_LENGTH+1, fp);
	if (length==0) {
		return;
	}
	else if (length > MAX_DATA_LENGTH) {
		return;
	}

	tempbuff[length] = '\0';
	/* Fill in send packet buffer */
	hstate->sendBuffer.valid = 1;
	hstate->sendBuffer.length = length;

	memset(hstate->sendBuffer.data, 0, sizeof(hstate->sendBuffer.data));
	for (i=0; i<length; i++) { /* Store tempbuff in payload of packet buffer */
		hstate->sendBuffer.data[i] = tempbuff[i];
	}
	
   fclose(fp);
   hostSilentInitTransmit(hstate, src);
   hostSilentTransmitPacket(hstate);
}

void hostSilentInitTransmit(hostState * hstate, int src) 
{
	if (hstate->sendBuffer.busy == 1) {
		return;
	}
	if (hstate->sendBuffer.valid == 0) {
		return;
	}

	hstate->sendBuffer.dstaddr = src;
	hstate->sendBuffer.busy = 1;
	hstate->sendBuffer.pos = 0;
}

void hostSilentTransmitPacket(hostState * hstate) 
{
	int i = 0;
	int length = hstate->sendBuffer.length - hstate->sendBuffer.pos;
	int error = 0;
	if (length > PAYLOAD_LENGTH) {
		length = PAYLOAD_LENGTH;	
	}
	
   hstate->sendPacketBuff.dstaddr = hstate->sendBuffer.dstaddr;
	hstate->sendPacketBuff.srcaddr = hstate->netaddr;
	hstate->sendPacketBuff.length = length;
	hstate->sendPacketBuff.type = 0;
   hstate->sendPacketBuff.end = 0;
	hstate->sendPacketBuff.start = 0;
	hstate->sendPacketBuff.root = 0;
   hstate->sendPacketBuff.distance = 0;
   
   if (hstate->sendBuffer.pos == 0) {
		hstate->sendPacketBuff.start = 1;
	}
	if (hstate->sendBuffer.valid) {
		hstate->sendPacketBuff.valid = 1;
	}

	for (i = 0; i < length; i++) {
		hstate->sendPacketBuff.payload[i] = 
			hstate->sendBuffer.data[i+hstate->sendBuffer.pos];
	}

	hstate->sendBuffer.pos += length;

	if (hstate->sendBuffer.pos >= hstate->sendBuffer.length) {
		hstate->sendBuffer.pos = 0;
		hstate->sendBuffer.busy = 0;
		hstate->sendPacketBuff.end = 1;
	}
	
	error = linkSend(&(hstate->linkout), &(hstate->sendPacketBuff));
	if (error == -1) {
		hostInitDataBuffer(&(hstate->sendBuffer));
		return;
	}
}


/* 
 * Initialize receive packet buffer 
 */ 

void hostInitRcvPacketBuff(packetBuffer * packetbuff)
{
packetbuff->valid = 0;
packetbuff->end = 0;
packetbuff->start = 0;
}

/*
 * Download the payload of the packet buffer into a 
 * file in the main directory
 */

void hostDownloadPacket(hostState * hstate, char fname[], char replymsg[]) 
{
char c;
FILE * fp;
char path[MAXBUFFER];  /* Path to the file */

/* Create a path to the file and then open it */

if (hstate->rcvBuffer.valid == 0) {
   strcpy(replymsg, "Download aborted: the receive packet buffer is empty");
   return;
}

if (hstate->maindirvalid == 0) {
   strcpy(replymsg, "Download aborted: the host's main directory is not yet chosen");
   return;
}

strcpy(path,"");
strcat(path, hstate->maindir);
strcat(path, "/");
strcat(path, fname);
printf("host:  path to the file: %s\n",path);
fp = fopen(path,"wb"); 

/* Download the packet buffer payload into the file */
if (hstate->rcvflag == 1) {
   fwrite(hstate->rcvBuffer.data,1,hstate->rcvBuffer.length,fp);
   memset(hstate->rcvBuffer.data, 0, sizeof(hstate->sendBuffer.data));
   hstate->rcvflag = 0;
   hostInitDataBuffer(&(hstate->rcvBuffer));
}

/* Message sent to the manager */
strcpy(replymsg, "Download successful");
fclose(fp);
} 

/* 
 * Clear the receive packet buffer
 */
void hostClearRcvFlg(hostState * hstate, char replymsg[])
{
hstate->rcvflag = 0;
hstate->rcvBuffer.valid = 0;
memset(hstate->rcvBuffer.data, 0, sizeof(hstate->sendBuffer.data));

/* Message to the manager */
strcpy(replymsg, "Host's packet received flag is cleared");
}

/* 
 * Change main directory of the host 
 */

void hostSetMainDir(hostState * hstate, char dirname[], char replymsg[])
{
strcpy(hstate->maindir, dirname);
hstate->maindirvalid = 1;

/* Message to the manager */
strcpy(replymsg, "Host's main directory name is changed");
}


void hostReqAddr(hostState * hstate, char hname[], char replymsg[])
{
   /* Packet to DNS */
   packetBuffer temp;
   strcpy(temp.payload, hname);
   temp.type = 4; /* Should be 4 */
   temp.valid = 1;
   temp.srcaddr = hstate->physid;    
   temp.dstaddr = 100;/* Address of DNS */
   temp.length = strlen(hname);
   temp.payload[temp.length] = '\0'; 
   linkSend(&(hstate->linkout), &temp);
   
   strcpy(replymsg, ""); //Clear reply buffer
   packetBuffer rcv;
   hostInitRcvPacketBuff(&rcv);
   int size = 0;
   size = linkReceive(&(hstate->linkin), &rcv);
   while(rcv.type != 5 || rcv.valid != 1 || rcv.dstaddr != hstate->netaddr)   {
      size = linkReceive(&(hstate->linkin), &rcv);
   }

   char buff[20];
   buff[0] = '\0'; //init to blank
   int2Ascii(buff, rcv.dnsaddr);

   if (rcv.dnsaddr != 255) {
    strcpy(replymsg, "Host address is ");
    strcat(replymsg, " ");
    strcat(replymsg, buff);
   } else {
    strcpy(replymsg, "Name did not match any host registered on DNS Server \n");
   }
   hostReplySend(&(hstate->manLink), "DISPLAY",replymsg);
}

void hostRequestFile(hostState * hstate, char filename[], int addr)  
{
   packetBuffer temp;
   strcpy(temp.payload, filename);
   temp.type = 6;
   temp.valid = 1;
   temp.srcaddr = hstate->physid;
   temp.dstaddr = addr;
   temp.length = strlen(filename);
   temp.payload[temp.length] = '\0';
   
   linkSend(&(hstate->linkout), &temp);
    
   char reply[1000];
   strcpy(reply, "File request sent..\n");
   hostReplySend(&(hstate->manLink), "DISPLAY", reply);
}

void hostSetName(hostState * hstate, char hname[], char replymsg[])
{
   /* Message to the manager */
   strcpy(replymsg, "Attempting to register name on DNS");
   /* Packet to DNS */
   packetBuffer temp;
   strcpy(temp.payload, hname);
   temp.type = 2; /* Should be 2 */
   temp.valid = 1;
   temp.srcaddr = hstate->physid;    
   temp.dstaddr = 100;/* Address of DNS */
   temp.length = strlen(hname);
   temp.payload[temp.length] = '\0'; 
   linkSend(&(hstate->linkout), &temp);
   hostReplySend(&(hstate->manLink), "DISPLAY",replymsg);
   
   strcpy(replymsg, ""); //Clear reply buffer
   packetBuffer rcv;
   hostInitRcvPacketBuff(&rcv);
   int size = 0;
   size = linkReceive(&(hstate->linkin), &rcv);
   while(rcv.type != 3 || rcv.valid != 1 || rcv.dstaddr != hstate->netaddr)   {
      size = linkReceive(&(hstate->linkin), &rcv);
   }

   if (rcv.flag == 1) {
    strcpy(replymsg, "Host name succesfully registered.");
   } else {
    strcpy(replymsg, "Host name failed to register.");
   }
   hostReplySend(&(hstate->manLink), "DISPLAY",replymsg);
}

/*
 * Get the host's state  
 * - host's physical ID
 * - host's main directory
 * - host's network address
 * - host's neighbor address
 * - host's receive flag
 * and create a message that has all this information to be sent
 * to the manager
 */
void hostGetHostState(hostState * hstate, managerLink * manLink, char replymsg[])
{
char word[1000];
char empty[7] = "Empty";

/* Create reply message */

replymsg[0] = '\0';  /* Clear reply message */

int2Ascii(word, hstate->physid);
appendWithSpace(replymsg, word);

if (hstate->maindirvalid==0) appendWithSpace(replymsg, empty);
else appendWithSpace(replymsg, hstate->maindir);

if (hstate->netaddr == EMPTY_ADDR) appendWithSpace(replymsg, empty);
else {
   int2Ascii(word, hstate->netaddr);
   appendWithSpace(replymsg, word);
}

if (hstate->nbraddr == EMPTY_ADDR) appendWithSpace(replymsg, empty);
else {
   int2Ascii(word, hstate->nbraddr);
   appendWithSpace(replymsg, word);
}

int2Ascii(word, hstate->rcvflag);
appendWithSpace(replymsg, word);

}

/* 
 * Initialize the state of the host 
 */
void hostInitState(hostState * hstate, int physid)
{
hstate->physid = physid;
hstate->maindirvalid = 0; /* main directory name is empty*/
hstate->hostnamevalid = 0; /* host name is initially invalid */
hstate->netaddr = physid; /* default address */  
hstate->nbraddr = EMPTY_ADDR;  
hstate->rcvPacketBuff.valid = 0;
hstate->rcvflag = 0;
hstate->rcvflag = 0;
hostInitDataBuffer(&(hstate->sendBuffer));
hostInitDataBuffer(&(hstate->rcvBuffer));
}

void hostInitDataBuffer(DataBuffer * buff) {
	buff->pos = 0;
	buff->busy = 0;
	buff->valid = 0;
	buff->length = 0;
	buff->start = 0;
	buff->end = 0;
}
