#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "csapp.h"

typedef struct {
    int status;
    char message[MAXLINE];
} Protocol;

#endif /* PROTOCOL_H */