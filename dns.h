#include "ntable.h"
#define NAME_LENGTH 100 

typedef struct { /* State of host */
   int   physid;              /* physical id */
   int   netaddr;             /* host's network address */
   char namebuff[50];
   packetBuffer rcvPacketBuff;   
   managerLink manLink;       /* Connection to the manager */
   LinkInfo linkin;           /* Incoming communication link */
   LinkInfo linkout;          /* Outgoing communication link */
   NameTable n_table; 
} dnsState;

void dnsMain(dnsState* dnsstate);
void dnsInit(dnsState * dnsstate, int physid);

