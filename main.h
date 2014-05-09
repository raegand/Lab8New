#ifndef MAIN_H_
#define MAIN_H_

#define PAYLOAD_LENGTH 200 /* Maximum payload size */
#define SOCKET 1
#define LINK 0

typedef struct { /* Packet buffer */
   int srcaddr;  /* Source address */
   int dstaddr;  /* Destination addres */
   int length;   /* Length of packet */
   char payload[PAYLOAD_LENGTH + 1];  /* Payload section */
   int type; /* type of packet */
   int root; /* current root data */
   int distance; 
   int dnsaddr; /* address response from DNS*/
   int valid;
   int parent; /* Sends flag "You are my parent" */
   int flag; /* flag for success or failure */
   /* for send buffer to indicate end */
   int start;
   int end;
} packetBuffer;

/* link num = link number
 * port one = outlink, port_two = inlink
 * link type = SOCKET or PIPE
 */

typedef struct {
   int link_num;
   int src, dst;
   int portsrc, portdst;
   int type;
} PortManager;



#endif

