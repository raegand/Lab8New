/* 
 * host.h 
 */

#ifndef HOST_H_
#define HOST_H_

#include "databuff.h"

#define NAME_LENGTH 100
#define DNS_LENGTH 50 

typedef struct { /* State of host */
   int   physid;              /* physical id */
   char  maindir[NAME_LENGTH]; /* main directory name */
   int   maindirvalid;        /* indicates if the main directory is empty */
   int   hostnamevalid;       /* indicates if the host name is valid */
   int   netaddr;             /* host's network address */
   int   nbraddr;             /* network address of neighbor */
   int   rcvflag;
   packetBuffer sendPacketBuff;  /* send packet buffer */
   packetBuffer rcvPacketBuff;   
   managerLink manLink;       /* Connection to the manager */
   LinkInfo linkin;           /* Incoming communication link */
   LinkInfo linkout;          /* Outgoing communication link */
   DataBuffer sendBuffer;
   DataBuffer rcvBuffer;
} hostState;

void hostMain(hostState * hstate);

void hostInit(hostState * hstate, int physid);

#endif

