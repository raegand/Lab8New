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

#define UPPER_NUM 57
#define LOWER_NUM 48
#define LOWER_UCL 65
#define UPPER_UCL 90 /* Upper limit Upper Case Letter */
#define LOWER_LCL 97/* Lower limite Lower Case Letter */
#define UPPER_LCL 122
#define CHAR_PER 46
#define CHAR_UNDSCR 95
#define TENMILLISEC 10000   /* 10 millisecond sleep */

void dnsInitState(dnsState* dstate, int physid); 
void dnsInitRcvPacketBuff(packetBuffer * packetbuff);
void dnsInitTransmit(dnsState * dstate, char word[], char replymsg[]);
void dnsTransmitSuccess(dnsState * dstate, int dstaddr);
void dnsTransmitFailure(dnsState * dstate, int dstaddr);
int checkName(char hname[], int length);
int isNumber(int x);
int isLowerCaseLetter(int x);
int isUpperCaseLetter(int x);

void writeDNSData(dnsState * dstate) 
{
   /* Writes switch data to file for debug purposes */
   DNSDebugTable(&(dstate->n_table), dstate->physid);
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
      /* Check if there is an incoming packet */
      length = linkReceive(&(dstate->linkin), &tmpbuff);
      if (tmpbuff.dstaddr == dstate->netaddr && tmpbuff.valid == 1 && tmpbuff.type == 2) {
         
         FILE * debug = fopen("DEBUG_DNS", "a");
         fprintf(debug, "Received DNS Packet! \n");
         fprintf(debug, "Data: %s \n", tmpbuff.payload);
         
         /* Do stuff with NTABLE + CHECK VALIDTY THEN RESPOND*/
         if(checkName(tmpbuff.payload, tmpbuff.length) == 1) {
            /* Passed name check, add to Ntable */
            UpdateNTableByAddress(&(dstate->n_table), tmpbuff.srcaddr, tmpbuff.payload);
            fprintf(debug, "Data passed test!\n");
            dnsTransmitSuccess(dstate, tmpbuff.srcaddr);  
         } else {
            dnsTransmitFailure( dstate, tmpbuff.srcaddr);
            fprintf(debug, "Data didn't pass test...\n");
         }
     
         fclose(debug); 
     }
     
      /* The host goes to sleep for 10 ms */
      writeDNSData(dstate); 
      usleep(TENMILLISEC);
   } /* End of while loop */
}

void dnsTransmitSuccess(dnsState * dstate, int dstaddr)
{
   packetBuffer response;
   response.type = DNSACK;
   response.srcaddr = dstate->physid;
   response.dstaddr = dstaddr;
   response.length = 1;
   response.flag = 1;
   response.valid = 1;
   linkSend(&(dstate->linkout), &response);
}

void dnsTransmitFailure(dnsState * dstate, int dstaddr)
{
   packetBuffer response;
   response.type = DNSACK;
   response.srcaddr = dstate->physid;
   response.dstaddr = dstaddr;
   response.length = 1;
   response.flag = 0;
   response.valid = 1;
   linkSend(&(dstate->linkout), &response);
}

int checkName(char hname[], int length) 
{
   int i = 0;
   for(i; i < length; i++) {
     int compare = (int)hname[i];
     if(compare != CHAR_PER && 
        compare != CHAR_UNDSCR && 
        (isNumber(compare) == 0) && 
        (isLowerCaseLetter(compare) == 0) && 
        (isUpperCaseLetter(compare) == 0)) {
      return 0; // Invalid Character Detected
     }
   }
   return 1;
}

int isNumber(int x)
{
   if(x >= LOWER_NUM && x <= UPPER_NUM) {
      return 1;
   } else {
      return 0;
   }
}

int isLowerCaseLetter(int x) 
{
   if(x >= LOWER_LCL && x <= UPPER_LCL) {
      return 1;
   } else {
      return 0;
   }
}

int isUpperCaseLetter(int x)
{
   if(x >= LOWER_UCL && x <= UPPER_UCL) {
      return 1;
   } else {
      return 0;
   }
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
   InitNTable(&(dnsstate->n_table));
   dnsstate->rcvPacketBuff.valid = 0;
}

