#include <libgen.h>
#include "protocol.h"

void send_request(int fd, Request *req, char *filename) {
    strcpy(req->filename, basename(filename));
    Rio_writen(fd, req, sizeof(Request));
}

int get_request(int fd, Request *req, char *filename) {
    if (Rio_readn(fd, req, sizeof(Request)) > 0) {
        strcpy(filename, req->filename);
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