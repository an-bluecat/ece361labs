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

char *strremove(char *str, const char *sub) {
    char *p, *q, *r;
    if ((q = r = strstr(str, sub)) != NULL) {
        size_t len = strlen(sub);
        while ((r = strstr(p = r + len, sub)) != NULL) {
            while (p < r)
                *q++ = *p++;
        }
        while ((*q++ = *p++) != '\0')
            continue;
    }
    return str;
}

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
		mod_client_ip(pac.source, (struct sockaddr *) &their_addr, 1);
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


void join(packet pac, char* addr, uint16_t port){

	printf("client ID: %s, want to join session: %s\n", pac.source, pac.data);

	// find session file in sessiondb folder
	char filename[100] = "./sessiondb/";
	strcat(filename, pac.data);
	int identified = cfileexists(filename);

	// add user into this session's file
    if(identified){

        printf("File %s exist\n",filename);

		// check if user is already in session
		char line[20];
		FILE *f_ = fopen(filename, "r");
		bool exist = false;
		while(fgets(line, sizeof line, f_)) {
			if(strstr(line, pac.source)) {
				exist = true;
			}
		}
		if (!exist) {
			FILE *f = fopen(filename, "a");
			fprintf(f, pac.source);
			fprintf(f, " ");
			fprintf(f, addr);
			fprintf(f, " ");
			fprintf(f, "%i", port);
			fprintf(f, "\n");
			fclose(f);
		}
		fclose(f_);
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
		strcpy(ack.data, pac.data);
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
	memset(qresult, 0, sizeof qresult); // flush
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
			
			printf("--%s--", path);
			strcat(qresult, fname);
			strcat(qresult, " has user: ");
			FILE* file = fopen(path, "r");
			char line[256];
			while (fgets(line, sizeof(line), file)) {
				char * c = strtok(line, " ");
				strcat(qresult, c);
				strcat(qresult, " ");
			}
			// add end of line for this file
			strcat(qresult, "\n");
			fclose(file);
		}

	}
	// printf("qresult:\n %s",qresult);
    closedir(dr);     

	// send the query result back to client
	packet ack;
	ack.type=QU_ACK;
	strcpy(ack.source,"empty");
	memset(ack.data, 0, sizeof ack.data); // flush
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


void leaveSession(packet pac){
	printf("the cleint <%s> wants to leave the session\n", pac.source);
	struct dirent *de;  // Pointer for directory entry 
  
    // opendir() returns a pointer of DIR type.  
    DIR *dr = opendir("./sessiondb"); 
  
    if (dr == NULL)  
    { 
        printf("Could not open current directory" ); 
        return 0; 
    } 
	
	// go through all files to find client id
    while ((de = readdir(dr)) != NULL){
		char fname[100];
		char path[100];
		strcpy(path, "./sessiondb/");
		strcpy(fname, de->d_name);
		strcat(path, fname);
		if((strcmp(fname,".")!=0) && (strcmp(fname,"..")!=0)){ // we don't want . and .. to be included in file names

			// delete line with user, use tmp file
			FILE* file = fopen(path, "r");
			FILE* tmp = fopen("./sessiondb/tmp.txt", "w");
			char line[256];
			int count = 0;
			while (fgets(line, sizeof(line), file)) {
				if (!strstr(line, pac.source)) {
					fputs(line, tmp);
				}
				count++;
			}
			remove(path);
			if (count == 1) {
				remove("./sessiondb/tmp.txt");
			} else {
				rename("./sessiondb/tmp.txt", path);
			}
			fclose(tmp);
			fclose(file);
		}

	}

}


void handleMsg(packet pac){
	printf("we are receiving text from client: %s\n", pac.source);
	printf("the message is: %s", pac.data);
	char msg[MAX_DATA];
	strcpy(msg, pac.data);
	
	struct dirent *de;  // Pointer for directory entry 
  
    // opendir() returns a pointer of DIR type.  
    DIR *dr = opendir("./sessiondb"); 
  
    if (dr == NULL)  
    { 
        printf("Could not open current directory" ); 
        return 0; 
    } 
	
	char target_path[100];

    while ((de = readdir(dr)) != NULL){
		char fname[100];
		char path[100];
		strcpy(path, "./sessiondb/");
		strcpy(fname, de->d_name);
		strcat(path, fname);
		if((strcmp(fname,".")!=0) && (strcmp(fname,"..")!=0)){ // we don't want . and .. to be included in file names

			FILE* file = fopen(path, "r");
			char line[256];
			while (fgets(line, sizeof(line), file)) {
				if (strstr(line, pac.source)) {
					fclose(file);
					strcpy(target_path, path);
					// break;
					// Do it here (for every session)
					FILE * target_file = fopen(target_path, "r");
					char line[256];
					while (fgets(line, sizeof(line), target_file)) {
						char * head = strtok(line, " ");
						char * ip = strtok(NULL, " ");
						int port = atoi(strtok(NULL, " "));
						char prefix[100];
						memset(prefix, 0, sizeof prefix);
						memset(pac.data, 0, sizeof pac.data);
						strcpy(prefix, de->d_name);
						strcat(prefix, ": ");
						strcat(prefix, msg);
						strcpy(pac.data, prefix);
						cast(ip, port, pac.data, QU_ACK);
					}
				}
			}
		}
	}
	
	// parse the target file
	// FILE * target_file = fopen(target_path, "r");
	// char line[256];
	// while (fgets(line, sizeof(line), target_file)) {
	// 	char * head = strtok(line, " ");
	// 	char * ip = strtok(NULL, " ");
	// 	int port = atoi(strtok(NULL, " "));
	// 	cast(ip, port, pac.data);
	// }
}

void cast(char* ip, int port, char* msg, int type) {
	// send the query result back to client
	packet m;
	m.type=type;
	strcpy(m.source,"empty");
	memset(m.data, 0, sizeof m.data); // flush
	strcpy(m.data, msg);
	m.size=strlen(m.data);
	char *ackStr=pacToStr(m);

	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	inet_pton(AF_INET, ip, &sa.sin_addr);

	//send ACK to client
	if ((numbytes = sendto(sockfd, ackStr, strlen(ackStr), 0, (struct sockaddr *)&sa, addr_len)) == -1) {
		perror("client: sendto");
		exit(1);
	}else{
		printf("sent\n");
	}
}


// helper, source: https://gist.github.com/jkomyno/45bee6e79451453c7bbdc22d033a282e
void get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
    switch(sa->sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                    s, maxlen);
            break;

        case AF_INET6:
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
                    s, maxlen);
            break;

        default:
            strncpy(s, "Unknown AF", maxlen);
    }
}

uint16_t get_port(const struct sockaddr *sa) {
	switch(sa->sa_family) {
        case AF_INET:
			return htons(((struct sockaddr_in *)sa)->sin_port);
            break;

        case AF_INET6:
            return htons(((struct sockaddr_in6 *)sa)->sin6_port);
            break;

        default:
            return NULL;
    }
}

void mod_client_ip(char * id, const struct sockaddr *sa, int add) {
	FILE* file = fopen("./userdb.txt", "r");
	FILE* tmp = fopen("./tmp.txt", "w");

	char line[256];
	while (fgets(line, sizeof(line), file)) {
		if (strstr(line, id)) {
			char new_line[256];
			if (add) {
				char suffix[96];
				char ip[64];
				get_ip_str(sa, ip, 64);
				u_int16_t port = get_port(sa);
				sprintf(suffix, " %s %i\n", ip, port);
				strcpy(new_line, line);
				new_line[strlen(new_line)-1] = '\0';
				strcat(new_line, suffix);
			} else {
				char * token = strtok(line, " ");
				strcpy(new_line, token);
				token = strtok(NULL, " ");
				strcat(new_line, " ");
				strcat(new_line, token);
				strcat(new_line, "\n");
			}
			fputs(new_line, tmp);
		} else {
			fputs(line, tmp);
		}
	}
	remove("./userdb.txt");
	rename("./tmp.txt", "./userdb.txt");
	fclose(tmp);
	fclose(file);

}

void invite_client(packet pac){
	FILE* file = fopen("./userdb.txt", "r");
	char line[256];
	char * id = strtok(pac.data, " ");
	char * msg = strtok(NULL, "\n");

	while (fgets(line, sizeof(line), file)) {
		if (strstr(line, id)) {
			strtok(line, " ");
			strtok(NULL, " ");
			char * ip = strtok(NULL, " ");
			if (!ip) {
				return;
			}
			char * port = strtok(NULL, "\n");

			cast(ip, atoi(port), msg, INV);
			break;
		}
	}
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
		char buf[MAXBUFLEN];
		buf[0]='\0';
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
		
		struct sockaddr * tmp = (struct sockaddr *)&their_addr; // convert into correct struct
		char ip[1024];
		get_ip_str(tmp, ip, 1024);
		uint16_t port = get_port(tmp);

		if(pac.type==LOGIN){
			login(pac);
		}else if(pac.type==JOIN){
			join(pac, ip, port);
		}else if(pac.type==NEW_SESS){
			create(pac);
		}else if(pac.type==QUERY){
			query(pac);
		}else if(pac.type==MESSAGE){
			handleMsg(pac);
		}else if(pac.type==LEAVE_SESS){
			leaveSession(pac);
		}else if(pac.type==INV){
			invite_client(pac);
		}
	}

	close(sockfd);

	return 0;
}