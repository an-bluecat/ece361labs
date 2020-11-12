

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


// helper
char* pacToStr(packet pac){
    char *result = malloc(1200*sizeof(char));

    sprintf(result, "%d", pac.type);
    memcpy(result+strlen(result), ";", sizeof(char));

    sprintf(result+strlen(result), "%d", pac.size);
    memcpy(result+strlen(result), ";", sizeof(char));

    // sprintf(result+strlen(result), "%s", pac.source);
    memcpy(result+strlen(result), pac.source, sizeof(char)*strlen(pac.source));
    memcpy(result+strlen(result), ";", sizeof(char));

    memcpy(result+strlen(result), pac.data, sizeof(char)*strlen(pac.data));

    return result;

}

void slice_str(const char * str, char * buffer, size_t start, size_t end) {
    size_t j = 0;
    for ( size_t i = start; i <= end; ++i ) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}

int main(){
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    /********** login ************/
	//start typing commands
	char command[MAXBUFLEN]; 
    fgets(command, MAXBUFLEN, stdin); 

    // split, take first: /login
    char *token = strtok(command, " "); 
    if(strcmp(token, "/login")!=0){
        printf("please login: /login <id> <pw> <serverIP> <port>\n");
    }
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
    // loginPac.size=strlen(loginPac.data);
    loginPac.size=2;
    char* loginStr=pacToStr(loginPac);
    printf("loginStr is :%s\n", loginStr);

    if ((numbytes = sendto(sockfd, loginStr, strlen(loginStr), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("client: sendto");
        exit(1);
    }






    /* close */
    freeaddrinfo(servinfo);
    // printf("deliver: sent %d bytes to %s\n", numbytes, argv[1]);
    close(sockfd);

    
    
    return 0;
}