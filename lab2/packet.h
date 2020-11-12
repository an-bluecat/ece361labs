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


