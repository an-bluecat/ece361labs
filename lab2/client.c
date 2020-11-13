

/*
IP: 128.100.13.155
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
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include "packet.h"


#define MAXBUFLEN 65535

char loggedInClient[200];
bool loggedIn=false;
char sessionID[200];
bool inSession=false;

int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;

void login(char* token){
    // get clientID from token
    token = strtok(NULL, " "); 
    if(token==NULL){ // end of string
        printf("not enough argument\n");
        exit(1);
    }
    char clientID[MAXBUFLEN];
    strcpy(clientID, token);
    printf("clientID is: %s\n", clientID);

    // get password from token
	token = strtok(NULL, " "); 
    if(token==NULL){
        printf("not enough argument\n");
        exit(1);
    }
    char* password=(char *) malloc(strlen(token));
    strcpy(password, token);
    printf("password is: %s\n", password);
    // server-IP
	token = strtok(NULL, " "); 
    if(token==NULL){
        printf("not enough argument\n");
        exit(1);
    }
    char* serverIP=(char *) malloc(strlen(token));
    strcpy(serverIP, token);
    printf("serverIP is: %s\n", serverIP);
    // servrerPort
	token = strtok(NULL, " \n"); // this one is different!!! has to be \n for the last
    if(token==NULL){
        printf("not enough argument\n");
        exit(1);
    }
    char* serverPort=(char *) malloc(strlen(token));
    strcpy(serverPort, token);
    printf("serverPort is: %s\n", serverPort);
    // printf("strlen: %ld", strlen(serverPort));

    // Sets the first num bytes of the block of memory pointed by ptr to the specified value (interpreted as an unsigned char).
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // ip4 or 6
    hints.ai_socktype = SOCK_DGRAM; // type
    // int getaddrinfo(const char *node, // e.g. "www.example.com" or IP
    // const char *service, // e.g. "http" or port number
    // const struct addrinfo *hints, // carries addr information
    // struct addrinfo **res);
    if ((rv = getaddrinfo(serverIP, serverPort, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        // try to make a socket on the first sucessful one
        // return file descriptor #
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("deliver: socket");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }

	char* IDPass = (char *) malloc(strlen(clientID)+ strlen(" ") + strlen(password) + 1);
    strcpy(IDPass, clientID);
    strcat(IDPass, " ");
	strcat(IDPass, password);
    printf("IDPass: %s\n", IDPass);

    packet loginPac;
    loginPac.type=LOGIN;
    strcpy(loginPac.source, clientID); // can't do assignment with char array
    strcpy(loginPac.data, password);
    loginPac.size=strlen(loginPac.data);
    char* loginStr=pacToStr(loginPac);
    printf("loginStr is :%s\n", loginStr);

    // send login info to server
    if ((numbytes = sendto(sockfd, loginStr, strlen(loginStr), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("client: sendto");
        exit(1);
    }

    // receive from server...
    char buf[MAXBUFLEN];
    socklen_t addr_len=sizeof(p->ai_addr);
    if (
        (numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *) &p->ai_addr, (unsigned int * restrict) &addr_len)) == -1
        ) {
		perror("recvfrom");
		exit(1);
	}
    printf("client: packet contains \"%s\"\n", buf);
    packet response=strToPac(buf);
    if(response.type==LO_ACK){ //receive LO_ACK, log in successful
        printf("login successful!\n");
        loggedIn=true;
        strcpy(loggedInClient, clientID);
    }else{
        printf("login unsuccessful! check your clientID and password\n");
    }
    // freeaddrinfo(servinfo);


    return;
}

void joinSession(char* token){

    // sessionID
	token = strtok(NULL, " \n"); // this one is different!!! has to be \n for the last
    if(token==NULL){
        printf("not enough argument\n");
        exit(1);
    }
    char* sessionID=(char *) malloc(strlen(token));
    strcpy(sessionID, token);
    printf("servrerPort is: %s\n", sessionID);

    packet joinPac;
    joinPac.type=JOIN;
    strcpy(joinPac.source, loggedInClient); //clientID
    strcpy(joinPac.data, sessionID); //data=sessionID
    joinPac.size=strlen(joinPac.data);
    char* joinStr=pacToStr(joinPac);
    printf("joinStr is :%s\n", joinStr);

    // send session info to server
    socklen_t addr_len=sizeof(p->ai_addr);
    if (
        // (numbytes = sendto(sockfd, joinStr, strlen(joinStr), 0, p->ai_addr, p->ai_addrlen)) == -1
        // (numbytes=send(sockfd, joinStr, strlen(joinStr), 0))==-1
        (numbytes = sendto(sockfd, joinStr, strlen(joinStr), 0 , (struct sockaddr *)&p->ai_addr, p->ai_addrlen)) == -1
        ) {
        perror("client: sendto");
        exit(1);
    }
    // receive from server...
    char buf[MAXBUFLEN];
    if (
        (numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *) &p->ai_addr, (unsigned int * restrict) &addr_len)) == -1
        ) {
		perror("recvfrom");
		exit(1);
	}
    printf("client: packet contains \"%s\"\n", buf);
    packet response=strToPac(buf);
    if(response.type==JN_ACK){ //receive LO_ACK, log in successful
        printf("join session successful!\n");
        inSession=true;
        strcpy(sessionID, sessionID);
    }else{
        printf("join session unsuccessful! check your session ID\n");
    }

    return;
}

void createSession(char* token){

    // sessionID
	token = strtok(NULL, " \n"); // this one is different!!! has to be \n for the last
    if(token==NULL){
        printf("not enough argument\n");
        exit(1);
    }
    char* sessionID=(char *) malloc(strlen(token));
    strcpy(sessionID, token);
    printf("servrerPort is: %s\n", sessionID);

    packet joinPac;
    joinPac.type=NEW_SESS;
    strcpy(joinPac.source, loggedInClient); //clientID
    strcpy(joinPac.data, sessionID); //data=sessionID
    joinPac.size=strlen(joinPac.data);
    char* joinStr=pacToStr(joinPac);
    printf("joinStr is :%s\n", joinStr);

    // send session info to server
    socklen_t addr_len=sizeof(p->ai_addr);
    if (
        (numbytes = sendto(sockfd, joinStr, strlen(joinStr), 0 , (struct sockaddr *)&p->ai_addr, p->ai_addrlen)) == -1
        ) {
        perror("client: sendto");
        exit(1);
    }
    // receive ACK from server 
    char buf[MAXBUFLEN];
    if (
        (numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *) &p->ai_addr, (unsigned int * restrict) &addr_len)) == -1
        ) {
		perror("recvfrom");
		exit(1);
	}
    printf("client: packet contains \"%s\"\n", buf);
    packet response=strToPac(buf);
    if(response.type==NS_ACK){ //receive NS_ACK, create session successful
        printf("create session successful!\n");
    }else{
        printf("create session: unsuccessful\n");
    }

    return;
}

void query(char* token){
    packet pac;
    pac.type=QUERY;
    strcpy(pac.source, loggedInClient); //clientID
    strcpy(pac.data, "empty");
    pac.size=strlen(pac.data);
    char* pacStr=pacToStr(pac);
    printf("pacStr is :%s\n", pacStr);

    // send session info to server
    socklen_t addr_len=sizeof(p->ai_addr);
    if (
        (numbytes = sendto(sockfd, pacStr, strlen(pacStr), 0 , (struct sockaddr *)&p->ai_addr, p->ai_addrlen)) == -1
        ) {
        perror("client: sendto");
        exit(1);
    }
    // receive from server...
    char buf[MAXBUFLEN];
    if (
        (numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *) &p->ai_addr, (unsigned int * restrict) &addr_len)) == -1
        ) {
		perror("recvfrom");
		exit(1);
	}
    printf("client: packet contains \"%s\"\n", buf);
    packet response=strToPac(buf);
    if(response.type==QU_ACK){ //receive LO_ACK, log in successful
        printf("query successful!\n");
        printf("here are the result: \n");
        printf("%s", response.data);
    }else{
        printf("query unsuccessful\n");
    }

    return;
}

void sendText(char* token){


    char* msg=(char *) malloc(strlen(token));
    strcpy(msg, token);
    printf("your message is: %s\n", msg);

    packet pac;
    pac.type=MESSAGE;
    strcpy(pac.source, loggedInClient); //clientID
    strcpy(pac.data, msg); //data=sessionID
    pac.size=strlen(pac.data);
    char* pacStr=pacToStr(pac);
    printf("pacStr is :%s\n", pacStr);

    // send message to server
    socklen_t addr_len=sizeof(p->ai_addr);
    if (
        (numbytes = sendto(sockfd, pacStr, strlen(pacStr), 0 , (struct sockaddr *)&p->ai_addr, p->ai_addrlen)) == -1
        ) {
        perror("client: sendto");
        exit(1);
    }


    return;
}


int main(){
    // int sockfd;
    // struct addrinfo hints, *servinfo, *p;
    // int rv;
    // int numbytes;
    
    while(1){
        //start typing commands
        printf(">");
        char command[MAXBUFLEN]; 
        fgets(command, MAXBUFLEN, stdin); 
        printf("received command: %s", command);
        // split, take first
        char *token = strtok(command, " "); 
        if(strcmp(token, "/quit\n")==0){ //quit
            printf("quitting...");
            exit(0);
        }
        if(loggedIn==false){ // have to login first
            if(strcmp(token, "/login")!=0){
                printf("please login: /login <id> <pw> <serverIP> <port>\n");
            }else{
                login(token);
                printf("loggedin=%d, your clientID is:%s\n", loggedIn, loggedInClient);
            }
        }else{ // if it's logged in

            if(strcmp(token, "/logout\n")==0){
                if(loggedIn==true){
                    loggedIn=false;
                    printf("You are logged out\n");
                }else{
                    printf("You are trying to log out , but no user is logged in\n");
                }
            }else if(strcmp(token, "/joinsession")==0){
                joinSession(token);
            }else if(strcmp(token, "/leavesession\n")==0){
                continue;
            }else if(strcmp(token, "/createsession")==0){
                createSession(token);
            }else if(strcmp(token, "/list\n")==0){
                query(token);
            }else{// send text
                if(!inSession){
                    printf("you are not in a session yet, can't send text");
                }else{
                    sendText(token);
                }
            }

        }

    }
	
    
    /* close */
    // freeaddrinfo(servinfo);
    // printf("deliver: sent %d bytes to %s\n", numbytes, argv[1]);
    close(sockfd);

    
    
    return 0;
}