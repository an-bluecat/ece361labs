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
#include <time.h>
#include <stdbool.h>
#include "packet.h"

// #define MYPORT "4950"
#define MAXBUFLEN 65535
bool tryTimeout=false;

// parse string to packet
packet strToPac(char* str){
    packet pac;
	char* ptr = str;

	char* tmp = str;
	while (*ptr != ';') {ptr++;}
	pac.total_frag = atoi(tmp);
	// printf("%d\n", pac.total_frag);

	*ptr = 0;
	tmp = ptr+1;
	ptr++;
	while (*ptr != ';') {ptr++;}
	pac.frag_no = atoi(tmp);

	*ptr = 0;
	tmp = ptr+1;
	ptr++;
	while (*ptr != ';') {ptr++;}
	pac.size = atoi(tmp);

	*ptr = 0;
	tmp = ptr+1;
	ptr++;
	while (*ptr != ';') {ptr++;}
	pac.filename = tmp;


	*ptr = 0;
	tmp = ptr+1;
	memset(pac.filedata, 0, sizeof(pac.filedata));
	memcpy(pac.filedata, tmp, pac.size*sizeof(char));
    return pac;
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        fprintf(stderr,"Error: please input <UDP listen port>\n");
        exit(1);
    }

	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;

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
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
    
    if((strcmp(buf, "ftp")==0)){
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

	/* receive package from client */
	FILE *f = NULL;
    char *fileName;
    bool firstPacket = true;
	char *buf_[MAXBUFLEN];
	while(1){
		memset(buf_, 0, sizeof(buf_));
		// receive from client the package
		if ((numbytes = recvfrom(sockfd, buf_, MAXBUFLEN , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom error in package transfer");
			exit(1);
		}

		// parse packet
		packet pac=strToPac(buf_);

		// first packet: create filename and open file
		if(firstPacket){
			// add "copy" to the back
			int slen=strlen(pac.filename)+strlen("copy-");
			fileName = (char*)malloc(sizeof(char*)*(slen));
            strcpy(fileName, "copy-");
			
			strcat(fileName, pac.filename);
			f = fopen(fileName, "w");
			if(!f){
				printf("can't create file");
				exit(1);
			}
			firstPacket=false;
			free(fileName);

			// printf("%s", buf_);
		}
		
		if(tryTimeout){
			while(1){
				int n=1;
				n++;
			}
		}
		

		// write to file
		fwrite(pac.filedata, pac.size, sizeof(char), f);

		// send ack to deliver
		if ((numbytes = sendto(sockfd, "ACK", strlen("ACK"), 0, (struct sockaddr *) &their_addr, addr_len)) == -1) {
            perror("deliver: sendto");
            exit(1);
        }

		// last packet
		if(pac.frag_no==pac.total_frag){
			fclose(f);
			break;
		}

	}

	close(sockfd);

	return 0;
}