#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "csapp.h"

#define BLOCK_SIZE 8196 // faster than 1024
#define MAX_MESSAGE_LEN 256
#define MAX_NAME_LEN 256
#define PORT 2121

typedef struct {
    int status;
    char message[MAX_MESSAGE_LEN];
} Protocol;

void set_response(Protocol *protocol, int status, char *message);

void get_response(Protocol *protocol, int *status, char *message);

#endif /* PROTOCOL_H */