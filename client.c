/*
 * client.c - A client for the concurrent server with pool of processes
 */

#include "csapp.h"
#define MAX_NAME_LEN 256
#define PORT 2121

int main(int argc, char **argv) {
    int clientfd;
    char *host, buf[MAXLINE];
    rio_t rio;
    char *filename;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <filename> \n", argv[0]);
        exit(0);
    }
    host = argv[1];
    filename = argv[2];
    /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = Open_clientfd(host, PORT);

    /*
     * At this stage, the connection is established between the client
     * and the server OS ... but it is possible that the server application
     * has not yet called "Accept" for this connection
     */
    printf("client connected to server OS\n");

    Rio_readinitb(&rio, clientfd);

    Rio_writen(clientfd, filename, strlen(filename)); // write to server
    Rio_writen(clientfd, "\n", 1); // write to server

    while (Rio_readlineb(&rio, buf, MAXLINE) > 0) { // read from server
        Fputs(buf, stdout); // write to stdout
    }

    Close(clientfd);
    exit(0);
}
