/*
** modified from http://beej.us/guide/bgnet/examples/listener.c
** talker.c -- a datagram "client" demo
other resources: https://www.geeksforgeeks.org/udp-server-client-implementation-c/
IP: 128.100.13.153
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
#include <bool.h>
#include "packet.h"

#define MAXBUFLEN 65535
// #define SERVERPORT "4950" // the port users will be connecting to
int main(int argc, char *argv[]){
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    if (argc != 3) {
        fprintf(stderr,"Error: please input <server address> <server port number>\n");
        exit(1);
    }


    // Sets the first num bytes of the block of memory pointed by ptr to the specified value (interpreted as an unsigned char).
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // ip4 or 6
    hints.ai_socktype = SOCK_DGRAM; // type
    // int getaddrinfo(const char *node, // e.g. "www.example.com" or IP
    // const char *service, // e.g. "http" or port number
    // const struct addrinfo *hints, // carries addr information
    // struct addrinfo **res);
    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
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

    // start type in ftp command
    char command[MAXBUFLEN]; 
    fgets(command, MAXBUFLEN, stdin); 
    // split
    char *token = strtok(command, " "); 
    if(strcmp(token, "ftp")!=0){
        printf("please enter: ftp <file name>\n");
    }
    token = strtok(NULL, " ");
    if(token==NULL){ // end of string
        printf("not enough argument\n");
        exit(1);
    }
    char filename[MAXBUFLEN];
    strcpy(filename, token);
    printf("looking for file: %s", filename);

    // trim filename
    char *tmp = filename;
    int len = strlen(tmp);

    while(isspace(tmp[len-1])) tmp[--len] = 0;
    while(*tmp && isspace(* tmp)) ++tmp, --len;

    memmove(filename, tmp, len+1);

    if(access(filename, F_OK) == -1){
        printf("no such file in the directory\n");
        exit(1);
    }

    // measure time
    clock_t t = clock();

    //send "ftp" to server
    if ((numbytes = sendto(sockfd, "ftp", strlen("ftp"), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("deliver: sendto");
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
    printf("listener: packet contains \"%s\"\n", buf);

    if (strcmp(buf, "yes")==0){
        printf("a file transfer can start\n");
    } else{
        printf("Error: didn't receive yes from server\n");
        exit(1);
    }

    // measure time
    t = clock() - t;
    printf("The round trip took %fms\n", (double)t);

    /* lab3: transfer the actual file */
    FILE *f=fopen(filename,"r");
    fseek(f, 0, SEEK_END); // seek to end of file
    int fsize = ftell(f); // get current file pointer
    fseek(f, 0, SEEK_SET); // seek back to beginning of file
    printf("fileLen: %d", fsize);
    int num_frag= floor((float)(fsize/1000))+1; //number of fragments needed
    int frag_num=1;
    while(1){
        //fill in packet information
        packet pac;
        pac.total_frag=num_frag;
        pac.frag_no=frag_num;
        if(frag_no==total_frag)){ //last one
            pac.size=fsize%1000;
        }else{
            pac.size=1000;
        }

        //convert package into a string
        //... code here
        char pacStr=pacToStr(pac);)

        // read the file into filedata
        fread((void*)packet.filedata, sizeof(char), pac.size, f);

        // sent to server
        if((numbytes = sendto(sockfd, pacStr, strlen(pacStr), 0 , p->ai_addr, p->ai_addrlen)) == -1) {
            printf("error for sending packet\n");
            exit(1);
        }

        
        // receive from server a "yes", for each package
        if (
            (numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *) &p->ai_addr, (unsigned int * restrict) &addr_len)) == -1
        ) {
            perror("recvfrom");
            exit(1);
        }
        // printf("listener: packet contains \"%s\"\n", buf);
        if (strcmp(buf, "yes")==0){
            printf("fragment ack received\n");
        } else{
            printf("Error: didn't receive yes from server for the fragment\n");
            exit(1);
        }

        // last one sent and received, break
        if (frag_num==num_frag){
            break;
        }
        // add 1, continue
        frag_num=frag_num+1;
    }





    /* close */
    freeaddrinfo(servinfo);
    // printf("deliver: sent %d bytes to %s\n", numbytes, argv[1]);
    close(sockfd);
    
    return 0;
}
