/*
 * client.c - A client for the concurrent server with pool of processes
 */

#include "csapp.h"

#define MAX_NAME_LEN 256
#define PORT 2121

int main(int argc, char **argv) {
    int clientfd;
    char *host, buf[MAXLINE];
    char filename[MAX_NAME_LEN];

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <filename> \n", argv[0]);
        exit(0);
    }
    host = argv[1];
    strncpy(filename, argv[2], strlen(argv[2]) + 1);

    clientfd = Open_clientfd(host, PORT);

    printf("client connected to server OS\n");

    Rio_writen(clientfd, filename, MAX_NAME_LEN); // write to server

    while (Rio_readn(clientfd, buf, MAXLINE) > 0) { // read from server
        Fputs(buf, stdout); // write to stdout
    }

    Close(clientfd);
    exit(0);
}
