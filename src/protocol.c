#include "protocol.h"

void set_response(Protocol *protocol, int status, char *message) {
    protocol->status = htonl(status);
    strcpy(protocol->message, message);
}

void get_response(Protocol *protocol, int *status, char *message) {
    *status = ntohl(protocol->status);
    strcpy(message, protocol->message);
}