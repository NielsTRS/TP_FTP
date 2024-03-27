#include <libgen.h>
#include "protocol.h"

void send_request(int fd, Request *req, char *filename, long starting_block) {
    strcpy(req->filename, basename(filename));
    req->starting_block = htonl(starting_block);
    Rio_writen(fd, req, sizeof(Request));
}

int get_request(int fd, Request *req, char *filename, long *starting_block) {
    if (Rio_readn(fd, req, sizeof(Request)) > 0) {
        strcpy(filename, req->filename);
        *starting_block = ntohl(req->starting_block);
        return 1;
    }
    return 0;
}

void send_response(int fd, Response *res, int status, char *message, long file_size, long block_number) {
    res->status = htonl(status);
    strcpy(res->message, message);
    res->file_size = htonl(file_size);
    res->block_number = htonl(block_number);
    Rio_writen(fd, res, sizeof(Response));
}

int get_response(int fd, Response *res, int *status, long *block_number, char *message, long *file_size) {
    if (Rio_readn(fd, res, sizeof(Response)) > 0) {
        *status = ntohl(res->status);
        *block_number = ntohl(res->block_number);
        strcpy(message, res->message);
        *file_size = ntohl(res->file_size);
        return 1;
    }
    return 0;
}

int get_slave_data(int fd, Slave *slave, char *ip, int *port) {
    if (Rio_readn(fd, slave, sizeof(Slave)) > 0) {
        strcpy(ip, slave->ip);
        *port = ntohl(slave->port);
        return 1;
    }
    return 0;
}