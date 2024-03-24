#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "csapp.h"

#define BLOCK_SIZE 8196 // faster than 1024
#define MAX_MESSAGE_LEN 256
#define MAX_NAME_LEN 256
#define PORT 2121

typedef struct {
    char filename[MAX_NAME_LEN];
} Request;

typedef struct {
    int status;
    char message[MAX_MESSAGE_LEN];
    long file_size;
} Response;

void send_request(int fd, Request *req, char *filename);

void get_request(int fd, Request *req, char *filename);

void send_response(Response *res, int status, char *message, char *filename);

void get_response(Response *res, int *status, char *message, long *file_size);

#endif /* PROTOCOL_H */