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

#define MAXBUFLEN 65535

int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;
struct sockaddr_storage their_addr;
socklen_t addr_len;

void login(packet pac){

	printf("client ID: %s, password: %s\n", pac.source, pac.data);

	//check in userdb is combo is valid
	FILE* file = fopen("userdb.txt", "r"); /* should check the result */
	char line[256];
	bool identified=false;
	while (fgets(line, sizeof(line), file)) {
		/* note that fgets don't strip the terminating \n, checking its
		presence would allow to handle lines longer that sizeof(line) */
		// printf("%s\n", line); 

		//check userdb
		char *ptr = strtok(line, " ");
		char *id=ptr;
		ptr = strtok(NULL, " \n");
		printf("id:%s", id);
		printf("password:%s\n", ptr);
		// compare with client data
		if(strcmp(pac.source,id)==0 && strcmp(pac.data,ptr)==0){
			identified=true;
			break;
		}
	}
	if(identified){ //send LO_ACK
		packet loAck;
		loAck.type=LO_ACK;
		strcpy(loAck.source,"empty");
		strcpy(loAck.data,"empty");
		loAck.size=strlen(loAck.data);
		char *loAckStr=pacToStr(loAck);
		if ((numbytes = sendto(sockfd, loAckStr, strlen(loAckStr), 0, (struct sockaddr *) &their_addr, addr_len)) == -1) {
			perror("client: sendto");
			exit(1);
		}else{
				printf("sent LO_ACK\n");
		}
	}else{	//send LO_NAK
		packet loAck;
		loAck.type=LO_NAK;
		strcpy(loAck.source,"empty");
		strcpy(loAck.data,"empty");
		loAck.size=strlen(loAck.data);
		char *loAckStr=pacToStr(loAck);
		if ((numbytes = sendto(sockfd, loAckStr, strlen(loAckStr), 0, (struct sockaddr *) &their_addr, addr_len)) == -1) {
			perror("client: sendto");
			exit(1);
		}else{
				printf("sent LO_NAK\n");
		}
	}
	fclose(file);
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        fprintf(stderr,"Error: please input <UDP listen port>\n");
        exit(1);
    }

	// int sockfd;
	// struct addrinfo hints, *servinfo, *p;
	// int rv;
	// int numbytes;
	// struct sockaddr_storage their_addr;
	// socklen_t addr_len;
	char buf[MAXBUFLEN];
	

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
	while(1){
		// receive from client.c
		printf("server: waiting to recvfrom...\n");
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN , 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
		
		printf("received the following: %s\n", buf);
		
		packet pac=strToPac(buf);
		
		if(pac.type==LOGIN){
			login(pac);
		}
	}
    
	


	close(sockfd);

	return 0;
}