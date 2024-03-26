#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "csapp.h"

#define BLOCK_SIZE 8196 // faster than 1024
#define MAX_MESSAGE_LEN 256
#define MAX_NAME_LEN 256
#define PORT 2121

typedef struct {
    char filename[MAX_NAME_LEN];
    long starting_block;
} Request;

typedef struct {
    int status;
    long block_number;
    char message[MAX_MESSAGE_LEN];
    long file_size;
} Response;

typedef struct {
    char buf[BLOCK_SIZE];
    ssize_t size;
} Block;

void send_request(int fd, Request *req, char *filename, long starting_block);

int get_request(int fd, Request *req, char *filename, long *starting_block);

void send_response(int fd, Response *res, int status, char *message, long file_size, long block_number);

int get_response(int fd, Response *res, int *status, long *block_number, char *message, long *file_size);

#endif /* PROTOCOL_H */