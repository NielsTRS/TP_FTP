#include <libgen.h>
#include "protocol.h"

void send_request(int fd, Request *req, char *user_input, long starting_block, int type) {
    if(type == FILE_TYPE) {
        strcpy(req->user_input, basename(user_input));
    } else {
        strcpy(req->user_input, user_input);
    }
    req->starting_block = htonl(starting_block);
    req->type = htonl(type);
    Rio_writen(fd, req, sizeof(Request));
}

int get_request(int fd, Request *req, char *user_input, long *starting_block, int *type) {
    if (Rio_readn(fd, req, sizeof(Request)) > 0) {
        strcpy(user_input, req->user_input);
        *starting_block = ntohl(req->starting_block);
        *type = ntohl(req->type);
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