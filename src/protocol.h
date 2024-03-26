#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "csapp.h"

#define BLOCK_SIZE 8196 // faster than 1024
#define MAX_MESSAGE_LEN 256
#define MAX_NAME_LEN 256
#define PORT 2121

// Request structure
typedef struct {
    char filename[MAX_NAME_LEN]; // filename
    long starting_block; // index of the block to start from
} Request;

// Response structure
typedef struct {
    int status; // status code (200, 404, etc.)
    long block_number; // number of blocks in total
    char message[MAX_MESSAGE_LEN]; // message coming with the status code
    long file_size; // size of the file in bytes
} Response;

// Block structure
typedef struct {
    char buf[BLOCK_SIZE]; // data buffer
    ssize_t size; // size of the data buffer
} Block;

void send_request(int fd, Request *req, char *filename, long starting_block);

int get_request(int fd, Request *req, char *filename, long *starting_block);

void send_response(int fd, Response *res, int status, char *message, long file_size, long block_number);

int get_response(int fd, Response *res, int *status, long *block_number, char *message, long *file_size);

#endif /* PROTOCOL_H */