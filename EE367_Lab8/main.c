#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "utilities.h"
#include "link.h"
#include "man.h"
#include "host.h"
#include "net.h"
#include "table.h"
#include "queue.h"
#include "switch.h"

#define EMPTY_ADDR  0xffff  /* Indicates that the empty address */
                             /* It also indicates that the broadcast address */
#define MAXBUFFER 1000
#define PIPEWRITE 1 
#define PIPEREAD  0
#define MAX_FILE_NAME 1000
#define MAX_WORD_SIZE 1000
#define MAX_LINKS 20

void main()
{
hostState hstate;             /* The host's state */
linkArrayType linkArray;
manLinkArrayType manLinkArray;
SwitchState s_state;

pid_t pid;  /* Process id */
int physid; /* Physical ID of host */
int i;
int k;

char filename[MAX_FILE_NAME];
char word[MAX_WORD_SIZE];
printf("Enter a filename: ");
scanf("%s",	filename);

int src[MAX_LINKS];
int dst[MAX_LINKS];
int count = 0;

int num_hosts = -1;
int num_links = -1;
int num_switches = -1;

FILE* file = fopen(filename, "r");
if (file != NULL) {
	char line[128];
	fgets(line, sizeof line, file);
	findWord(word, line, 1);
	num_hosts = ascii2Int(word);
	findWord(word, line, 2);
	num_links = ascii2Int(word);
	findWord(word, line, 3);
	num_switches = ascii2Int(word);

	while (fgets(line, sizeof line, file) != NULL) {
		findWord(word, line, 1);
		src[count] = ascii2Int(word);
		findWord(word, line, 2);
		dst[count] = ascii2Int(word);
		count++;
	}
	fclose(file);
} else {
	perror(filename);
	return;
}

if (num_hosts == -1 || num_links == -1 || num_switches == -1 ||
	count != num_links/2 || num_links%2 != 0) {
	printf("invalid information in file\n");
	return;
}

manLinkArray.link = (managerLink*)malloc(num_hosts*sizeof(managerLink));
linkArray.link = (LinkInfo*)malloc(num_links*sizeof(LinkInfo));
s_state.link_in = (LinkInfo*)malloc(num_hosts*sizeof(LinkInfo));
s_state.link_out = (LinkInfo*)malloc(num_hosts*sizeof(LinkInfo));

/* 
 * Create nonblocking (pipes) between manager and hosts 
 * assuming that hosts have physical IDs 0, 1, ... 
 */
manLinkArray.numlinks = num_hosts;
netCreateConnections(& manLinkArray);

/* Create links between nodes but not setting their end nodes */
linkArray.numlinks = num_links;
netCreateLinks(& linkArray);

/* Set the end nodes of the links */

netSetNetworkTopology(& linkArray, src, dst);

/* Create nodes and spawn their own processes, one process per node */ 
/* physid corresponds to the unique host id */
for (physid = 0; physid < num_hosts; physid++) {

	pid = fork();

	if (pid == -1) { printf("Error:  the fork() failed\n");
		return;
	}
	else if (pid == 0) { /* The child process -- a host node */

		hostInit(&hstate, physid);              /* Initialize host's state */

		/* Initialize the connection to the manager */ 
		hstate.manLink = manLinkArray.link[physid];

		/* 
		 * Close all connections not connect to the host
		 * Also close the manager's side of connections to host
		 */
		netCloseConnections(& manLinkArray, physid);

		/* Initialize the host's incident communication links */

		k = netHostOutLink(&linkArray, physid); /* Host's outgoing link */
		hstate.linkout = linkArray.link[k];

		k = netHostInLink(&linkArray, physid); /* Host's incoming link */
		hstate.linkin = linkArray.link[k];

		/* Close all other links -- not connected to the host */
		netCloseHostOtherLinks(& linkArray, physid);

		/* Go to the main loop of the host node */
		hostMain(&hstate);
		free(manLinkArray.link);
		free(s_state.link_in);
		free(s_state.link_out);
		free(linkArray.link);
	}  
}

for (physid = num_hosts; physid < num_hosts + num_switches; physid++) {
	pid = fork();
	if (pid == -1) {
		printf("Error: the fork() failed\n");
		return;
	} else if (pid == 0) {
		switchInit(&s_state, physid);
		/* close all manager links, switches don't need them */
		netCloseAllManLinks(&manLinkArray);
		/* initlialize connections to the switch */
		netSwitchLinks(&linkArray, &s_state, physid);
		/* close all other connections that are not connected to switch */
		netCloseSwitchOtherLinks(&linkArray, physid);

		switchMain(&s_state);
		free(s_state.link_in);
		free(s_state.link_out);
		free(linkArray.link);
		free(manLinkArray.link);
	}
}

/* Manager */

/* 
 * The manager is connected to the hosts and doesn't
 * need the links between nodes
 */

/* Close all links between nodes */
netCloseLinks(&linkArray);

/* Close the host's side of connections between a host and manager */
netCloseManConnections(&manLinkArray);

/* Go to main loop for the manager */
manMain(& manLinkArray);
free(s_state.link_in);
free(s_state.link_out);
free(linkArray.link);
free(manLinkArray.link);


/* 
 * We reach here if the user types the "q" (quit) command.
 * Now if we don't do anything, the child processes will continue even
 * after we terminate the parent process.  That's because these
 * child proceses are running an infinite loop and do not exit 
 * properly.  Since they have no parent, and no way of controlling
 * them, they are called "zombie" processes.  Actually, to get rid
 * of them you would list your processes using the LINUX command
 * "ps -x".  Then kill them one by one using the "kill" command.  
 * To use the kill the command just type "kill" and the process ID (PID).
 *
 * The following system call will kill all the children processes, so
 * that saves us some manual labor
 */
kill(0, SIGKILL); /* Kill all processes */
}




