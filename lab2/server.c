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
#include <dirent.h> 

#define MAXBUFLEN 65535

int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;
struct sockaddr_storage their_addr;
socklen_t addr_len;

int cfileexists(const char * filename){
    /* try to open file to read */
    FILE *file;
    if (file = fopen(filename, "r")){
        fclose(file);
        return 1;
    }
    return 0;
}

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

	return;
}


void join(packet pac){

	printf("client ID: %s, want to join session: %s\n", pac.source, pac.data);

	// find session file in sessiondb folder
	char filename[100] = "./sessiondb/";
	strcat(filename, pac.data);
	int identified = cfileexists(filename);

	// add user into this session's file
    if(identified){
        printf("File %s exist\n",filename);
		FILE *f = fopen(filename, "w");
		fprintf(f, pac.source);
		fprintf(f, " ");
		fclose(f);
	}
    else{
        printf("File %s does not exist\n",filename);
	}

	// send ACK back to client
	if(identified){ //send JN_ACK
		packet jnAck;
		jnAck.type=JN_ACK;
		strcpy(jnAck.source,"empty");
		strcpy(jnAck.data, pac.data); // pack sessionID into jnAck.data
		jnAck.size=strlen(jnAck.data);
		char *jnAckStr=pacToStr(jnAck);
		//send ACK to client
		if ((numbytes = sendto(sockfd, jnAckStr, strlen(jnAckStr), 0, (struct sockaddr *) &their_addr, addr_len)) == -1) {
			perror("client: sendto");
			exit(1);
		}else{
			printf("sent JN_ACK\n");
		}
	}else{	//send LO_NAK
		packet jnAck;
		jnAck.type=JN_NAK;
		strcpy(jnAck.source,"empty");
		strcpy(jnAck.data, "no such session exist in server"); // pack sessionID into jnAck.data
		jnAck.size=strlen(jnAck.data);
		char *jnAckStr=pacToStr(jnAck);
		//send ACK to client
		if ((numbytes = sendto(sockfd, jnAckStr, strlen(jnAckStr), 0, (struct sockaddr *) &their_addr, addr_len)) == -1) {
			perror("client: sendto");
			exit(1);
		}else{
				printf("sent JN_NAK\n");
		}
	}

	return;
}

void create(packet pac){

	printf("client ID: %s, want to create session: %s\n", pac.source, pac.data);

	// find session file in sessiondb folder
	char filename[100] = "./sessiondb/";
	strcat(filename, pac.data);
	int identified = cfileexists(filename);

	// create a file if not exist, and send ACK
    if(!identified){ 
		// not exist, create file
		FILE *f = fopen(filename, "w");
		fclose(f);
		// send ACK
		packet ack;
		ack.type=NS_ACK;
		strcpy(ack.source,"empty");
		strcpy(ack.data, pac.data); // pack sessionID into jnAck.data
		ack.size=strlen(ack.data);
		char *ackStr=pacToStr(ack);
		//send ACK to client
		if ((numbytes = sendto(sockfd, ackStr, strlen(ackStr), 0, (struct sockaddr *) &their_addr, addr_len)) == -1) {
			perror("client: sendto");
			exit(1);
		}else{
				printf("sent NS_ACK\n");
		}
	}
    else{
		// send NAK
		packet ack;
		ack.type=LO_NAK;
		strcpy(ack.source,"empty");
		strcpy(ack.data, "session already exist");
		ack.size=strlen(ack.data);
		char *ackStr=pacToStr(ack);
		//send ACK to client
		if ((numbytes = sendto(sockfd, ackStr, strlen(ackStr), 0, (struct sockaddr *) &their_addr, addr_len)) == -1) {
			perror("client: sendto");
			exit(1);
		}else{
				printf("sent NS_ACK\n");
		}
	}

	return;
}

void query(packet pac){
	char qresult[500];
	// strcpy(qresult, " ");
	struct dirent *de;  // Pointer for directory entry 
  
    // opendir() returns a pointer of DIR type.  
    DIR *dr = opendir("./sessiondb"); 
  
    if (dr == NULL)  
    { 
        printf("Could not open current directory" ); 
        return 0; 
    } 
	
	// open
    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html for readdir() 
    while ((de = readdir(dr)) != NULL){
		char fname[100];
		char path[100];
		strcpy(path, "./sessiondb/");
		strcpy(fname, de->d_name);
		strcat(path, fname);
		if((strcmp(fname,".")!=0) && (strcmp(fname,"..")!=0)){ // we don't want . and .. to be included in file names
			strcat(qresult, fname);
			strcat(qresult, " has user: ");
			FILE* file = fopen(path, "r");
			char line[256];
			fgets(line, sizeof(line), file);
			strcat(qresult, line);
			// add end of line for this file
			strcat(qresult, " \n");
			fclose(file);
		}

	}
	printf("qresult:\n %s\n",qresult);
    closedir(dr);     

	// send the query result back to client
	packet ack;
	ack.type=QU_ACK;
	strcpy(ack.source,"empty");
	strcpy(ack.data, qresult);
	ack.size=strlen(ack.data);
	char *ackStr=pacToStr(ack);
	//send ACK to client
	if ((numbytes = sendto(sockfd, ackStr, strlen(ackStr), 0, (struct sockaddr *) &their_addr, addr_len)) == -1) {
		perror("client: sendto");
		exit(1);
	}else{
			printf("sent QU_ACK\n");
	}

}

void handleMsg(packet pac){
	printf("we are receiving from client: %s", pac.source);
	printf("the message is: %s", pac.data);
	return;
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
		}else if(pac.type==JOIN){
			join(pac);
		}else if(pac.type==NEW_SESS){
			create(pac);
		}else if(pac.type==QUERY){
			query(pac);
		}else if(pac.type==MESSAGE){
			handleMsg(pac);
		}
	}

	close(sockfd);

	return 0;
}