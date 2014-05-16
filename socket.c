//
//  socket.c
//  ee367_final_lab
//
//  Created by Joseph Felix on 5/1/14.
//  Copyright (c) 2014 Joseph Phillip Felix Jr. All rights reserved.
//

#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define BACKLOG 10
#define MAXDATASIZE 256

void sigchld_handler(int s)
{
   while(waitpid(-1, NULL, WNOHANG) > 0);
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void socketSend(char payload[], int length, int port)
{
    // Variables to declare
    // Client
    int sockfd, numbytes; // BOTH
    char buf[PACKET_SIZE];
    struct addrinfo hints, *servinfo, *p;  // BOTH
    int rv; // BOTH
    char s[INET6_ADDRSTRLEN];   // BOTH
    int i;  // BOTH
    
    char argv[] = "localhost";
    /*********** Ask user for server address ************/
    /*********** This can be hardcoded       ************/
    /************ End of server address *************/
    
	memset(&hints, 0, sizeof hints);    // BOTH
	hints.ai_family = AF_UNSPEC;        // BOTH
	hints.ai_socktype = SOCK_STREAM;    // BOTH
    
    /****** BOTH *****/
	if ((rv = getaddrinfo(argv[1], port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
    
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
       if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
           close(sockfd);
           perror("client: connect");
           continue;
       }
    break;
    
   }
	
   if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
    
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
	printf("client: connecting to %s\n", s);
    
	freeaddrinfo(servinfo); // all done with this structure
    if (send(sockfd, payload, length-1, 0) == -1) {
        perror("send failed");
        return false;
    }
    close(sockfd);
    exit(1);
}

//*
if(!fork() && pm[z].port_server!=0)
{
    // Server
    int server_sockfd, server_numbytes; // BOTH
    int server_newfd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo server_hints, *server_servinfo, *server_p;  // BOTH
    struct sockaddr_storage server_their_addr; // connector's address information
    socklen_t server_sin_size;  // SERVER
    struct sigaction server_sa; // SERVER
    int server_yes=1; // SERVER
    char server_s[INET6_ADDRSTRLEN]; // BOTH
    int server_rv; // BOTH
    char server_commandbuf[PACKET_SIZE]; // CLIENT buf same
    int server_i, server_j;   // BOTH counters
    char word[1000];
    
    memset(&server_hints, 0, sizeof server_hints);    // BOTH
	server_hints.ai_family = AF_UNSPEC;        // BOTH
	server_hints.ai_socktype = SOCK_STREAM;    // BOTH
	server_hints.ai_flags = AI_PASSIVE;        // use my IP SERVER ONLY
    
    /********* BOTH *********/
	if ((server_rv = getaddrinfo(NULL, pm[z].port_server, &server_hints, &server_servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(server_rv));
		return;
	}
	// loop through all the results and bind to the first we can
	for(server_p = server_servinfo; server_p != NULL; server_p = server_p->ai_next) {
		if ((server_sockfd = socket(server_p->ai_family, server_p->ai_socktype,
                                    server_p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
        /***********************/
        
        /******* SERVER ********/
		if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &server_yes,
                       sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		if (bind(server_sockfd, server_p->ai_addr, server_p->ai_addrlen) == -1) {
			close(server_sockfd);
			perror("server: bind");
			continue;
		}
        /**********************/
		break;
	}
    /*********** BOTH **********/
	if (server_p == NULL)  {

		fprintf(stderr, "server: failed to bind\n");
		return;
	}
    
	freeaddrinfo(server_servinfo); // all done with this structure
    /***************************/
    
    
    /******* SERVER *******/
	if (listen(server_sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
    
	server_sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&server_sa.sa_mask);
	server_sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &server_sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
    /**********************/
    
	printf("server: waiting for connections...\n");
    while(1)
    {  // main accept() loop
        server_sin_size = sizeof server_their_addr;
        server_newfd = accept(server_sockfd, (struct sockaddr *)&server_their_addr, &server_sin_size);
        if (server_newfd == -1) {
            perror("accept failed");
            continue;
        }
        inet_ntop(server_their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&server_their_addr),
                  server_s, sizeof server_s);
        printf("server: got connection from %s\n", server_s);
        
        if (!fork()) { // this is the child process
            close(server_sockfd); // child doesn't need the listener
            if ((server_numbytes = recv(server_newfd, server_commandbuf, PACKET_SIZE,0)) == -1) {
                perror("recv failed");
                exit(1);
            }
            server_commandbuf[server_numbytes] = '\0';
            printf("server: received command '%s' \n", server_commandbuf);
            
            // convert commandbuf packet
            /* MANDATORY */
            findWord(word, buffer, 1); /* Destination address */
            pbuff.dstaddr = ascii2Int(word);
            
            findWord(word, buffer, 2); /* Source address */
            pbuff.srcaddr = ascii2Int(word);
            
            findWord(word, buffer, 3); /* Packet Type */
            pbuff.type = ascii2Int(word);
            
            switch(pbuff.type) {
                case 0:
                    findWord(word, buffer, 4); /* Length */
                    pbuff.length = ascii2Int(word);
                    
                    findWord(word, buffer, 5); /* End */
                    pbuff.end = ascii2Int(word);
                    
                    findWord(word, buffer, 6); /* Start */
                    pbuff.start = ascii2Int(word);
                    
                    findWord(word, buffer, 7); /* Payload */
                    break;
                case 1:
                    findWord(word, buffer, 4);
                    pbuff.root = ascii2Int(word);  /* Current Root  */
                    
                    findWord(word, buffer, 5);
                    pbuff.distance = ascii2Int(word); /* Current Distance  */
                    
                    findWord(word, buffer, 6); /* Length */
                    pbuff.length = ascii2Int(word);
                    
                    break;
                case 3:
                    findWord(word, buffer, 4);
                    pbuff.flag = ascii2Int(word); /* Current Distance  */
                    
                    findWord(word, buffer, 5); /* Length */
                    pbuff.length = ascii2Int(word);
                    break;
                case 4: /* Same as DNS PACKET, Length then Payload  */
                case 5: /* Same as DNS PACKET, Length then Payload  */
                case 2:
                    findWord(word, buffer, 4); /* Length */
                    pbuff.length = ascii2Int(word);
                    /* NAME is in PAYLOAD */
                    break;
            }
            
            /*
             * We will transform the payload so that
             *
             *  Symbols 'a', 'b', ..., 'p' converts to the
             *  4-bits 0000, 0001,..., 1111
             *  Each pair of symbols converts to a byte.
             *  For example, 'ac' converts to 00000010
             *  Note the first symbol is the high order bits
             *  and the second symbol is the low order bits
             */
            
            for (k = 0; k < pbuff.length; k++){
                highbits = word[2*k]-'a';
                lowbits = word[2*k+1]-'a';
                highbits = highbits * 16; /* Shift to the left by 4 bits */
                pbuff.payload[k] = highbits + lowbits;
            } /* end of for */
            pbuff.payload[k] = '\0';
            pbuff.valid=1;
        } /* end of if */
        else { /* Not a packet */
            pbuff.valid=0;
        }
    }
    
    close(server_newfd);  // parent doesn't need this
    exit(1);
}

