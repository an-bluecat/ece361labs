

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

int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;

void login(char* token, int *sockfd){


    // clientID
    token = strtok(NULL, " "); 
    if(token==NULL){ // end of string
        printf("not enough argument\n");
        exit(1);
    }
    char clientID[MAXBUFLEN];
    strcpy(clientID, token);
    printf("clientID is: %s\n", clientID);
    // password
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
    printf("servrerPort is: %s\n", serverPort);
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
        return 1;
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

    //send to server IDPass: <clientID> <Password>
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
    freeaddrinfo(servinfo);


    return;
}

void slice_str(const char * str, char * buffer, size_t start, size_t end) {
    size_t j = 0;
    for ( size_t i = start; i <= end; ++i ) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
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

        // split, take first
        char *token = strtok(command, " "); 
        if(strcmp(token, "/quit")==0){ //quit
            printf("quitting...");
            exit(0);
        }
        if(loggedIn==false){ // have to login first
            if(strcmp(token, "/login")!=0){
                printf("please login: /login <id> <pw> <serverIP> <port>\n");
            }else{
                login(token, &sockfd);
                printf("loggedin=%d, your clientID is:%s", loggedIn, loggedInClient);
            }
        }else{
            if(strcmp(token, "/logout")==0){
                if(loggedIn==true){
                    loggedIn=false;
                }else{
                    printf("You are trying to log out , but no user is logged in\n");
                }
            }else if(strcmp(token, "/joinsession")==0){

            }else if(strcmp(token, "/leavesession")==0){
                
            }else if(strcmp(token, "/createsession")==0){
                
            }else if(strcmp(token, "/list")==0){
                
            }else{// send text
                continue;
            }

        }

    }
	
    
    /* close */
    // freeaddrinfo(servinfo);
    // printf("deliver: sent %d bytes to %s\n", numbytes, argv[1]);
    close(sockfd);

    
    
    return 0;
}