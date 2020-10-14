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



char* pacToStr(packet pac){
    int size=sizeof(packet);
    printf("size is %d", size);
    char result=(char*) malloc(size);
    return &result;
}