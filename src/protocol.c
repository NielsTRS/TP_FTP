#include <libgen.h>
#include "protocol.h"

void send_request(int fd, Request *req, char *filename) {
    strcpy(req->filename, basename(filename));
    Rio_writen(fd, req, sizeof(Request));
}

void get_request(int fd, Request *req, char *filename) {
    if (Rio_readn(fd, req, sizeof(Request)) > 0) {
        strcpy(filename, req->filename);
    }
}

void send_response(Response *res, int status, char *message, char *filename) {
    struct stat st;
    res->status = htonl(status);
    strcpy(res->message, message);
    if (filename != NULL) {
        stat(filename, &st);
        res->file_size = htonl(st.st_size);
    }
}

void get_response(Response *res, int *status, char *message, long *file_size) {
    *status = ntohl(res->status);
    strcpy(message, res->message);
    *file_size = ntohl(res->file_size);
}