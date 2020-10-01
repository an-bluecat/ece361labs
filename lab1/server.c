/*
** modified from http://beej.us/guide/bgnet/examples/listener.c and https://www.geeksforgeeks.org/udp-server-client-implementation-c/
listener.c -- a datagram sockets "server" demo

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// #define MYPORT "4950"
#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        fprintf(stderr,"Error: please input <UDP listen port>\n");
        exit(1);
    }
    // int MYPORT = atoi(argv[1]);

	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	// char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {// socket(domain, type, protocal)
			perror("server: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

    // receive "ftp" from deliver.c
	printf("server: waiting to recvfrom...\n");
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
    // buf[numbytes] = '\0';
	// printf("listener: packet contains \"%s\"\n", buf);
    // ?????????????????????????????????????????? why str cmp doesn't work? 
    // strcmp(buf, "ftp") won't work
    
    if((strncmp(buf, "ftp",3)==0)){
        if ((numbytes = sendto(sockfd, "yes", strlen("yes"), 0, (struct sockaddr *) &their_addr, addr_len)) == -1) {
            perror("deliver: sendto");
            exit(1);
        }else{
            printf("sent yes\n");
        }
    }else{
        if ((numbytes = sendto(sockfd, "no", strlen("no"), 0, (struct sockaddr *) &their_addr, addr_len)) == -1) {
            perror("deliver: sendto");
            exit(1);
        }else{
            printf("sent no\n");
        }
    }
	// printf("listener: got packet from %s\n",
	// 	inet_ntop(their_addr.ss_family,
	// 		get_in_addr((struct sockaddr *)&their_addr),
	// 		s, sizeof s));
	// printf("listener: packet is %d bytes long\n", numbytes);
	// buf[numbytes] = '\0';
	// printf("listener: packet contains \"%s\"\n", buf);

	close(sockfd);

	return 0;
}