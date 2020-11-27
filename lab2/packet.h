#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#define MAX_NAME 200
#define MAX_DATA 500

typedef struct message {
    unsigned int type; //messagetype
    unsigned int size; //size
    unsigned char source[MAX_NAME]; // client ID
    unsigned char data[MAX_DATA]; //data
} packet;

char *my_itoa(int num, char *str)
{
        if(str == NULL)
        {
                return NULL;
        }
        sprintf(str, "%d", num);
        return str;
}


char* pacToStr(packet pac){
    char *result = malloc(1200*sizeof(char));
    char type[5];
    my_itoa(pac.type, type);
    char size[5];
    my_itoa(pac.size, size);
    strcpy(result, type);
    strcat(result, ";");
    strcat(result, size);
    strcat(result, ";");
    strcat(result, pac.source);
    strcat(result, ";");
    strcat(result, pac.data);
    return result;
}

// parse string to packet
packet strToPac(char* str){
	packet pac;

	char* token = strtok(str, ";"); 
    if(token==NULL){
        printf("not enough argument\n");
        exit(1);
    }
	pac.type = atoi(token);

	token = strtok(NULL, ";"); 
    if(token==NULL){
        printf("not enough argument\n");
        exit(1);
    }
	pac.size = atoi(token);

	token = strtok(NULL, ";"); 
    strcpy(pac.source, token);
    // printf("password is: %s\n", pac);

	token = strtok(NULL, "\0"); 
    strcpy(pac.data, token);

    return pac;
}


#define LOGIN 0
#define LO_ACK 1
#define LO_NAK 2
#define EXIT 3
#define JOIN 4
#define JN_ACK 5
#define JN_NAK 6
#define LEAVE_SESS 7
#define NEW_SESS 8
#define NS_ACK 9
#define MESSAGE 10
#define QUERY 11
#define QU_ACK 12
#define INV 13


