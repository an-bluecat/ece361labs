#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

typedef struct packet{
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    char filedata[1000];
} packet;


// encoude packet to string
char* pacToStr(packet pac){
    char result[1200];
    // int size=sizeof(packet);
    // printf("size is %d", size);
    // result=(char*) malloc(size);
    return result;
}

// parse string to packet
packet strToPac(char*){
    packet pac;
    return pac;
}