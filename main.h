#ifndef MAIN_H_
#define MAIN_H_

#define PAYLOAD_LENGTH 200 /* Maximum payload size */

typedef struct { /* Packet buffer */
   int srcaddr;  /* Source address */
   int dstaddr;  /* Destination addres */
   int length;   /* Length of packet */
   char payload[PAYLOAD_LENGTH + 1];  /* Payload section */
   int type; /* type of packet */
   int root; /* current root data */
   int distance; 
   int valid;
   int parent; /* Sends flag "You are my parent" */
   int flag; /* flag for success or failure */
   /* for send buffer to indicate end */
   int start;
   int end;
} packetBuffer;


#endif

