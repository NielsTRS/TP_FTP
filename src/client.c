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
    FILE *file;
    ssize_t bytes_read;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <filename> \n", argv[0]);
        exit(0);
    }
    host = argv[1];
    strncpy(filename, argv[2], strlen(argv[2]) + 1);

    clientfd = Open_clientfd(host, PORT);

    printf("client connected to server OS\n");

    Rio_writen(clientfd, filename, MAX_NAME_LEN); // write to server

    while ((bytes_read = Rio_readn(clientfd, buf, MAXLINE)) > 0) { // Read from server

        if(strncmp(buf, "404", 3) == 0) {
            fprintf(stderr, "Error opening file %s\n", filename);
            Close(clientfd);
            exit(1);
        }

        file = fopen(filename, "wb"); // Open or create a local file for writing in binary mode
        if (file == NULL) {
            fprintf(stderr, "Error opening local file %s\n", filename);
            Close(clientfd);
            exit(1);
        }
        if (fwrite(buf, 1, bytes_read, file) != bytes_read) { // Write to local file
            fprintf(stderr, "Error writing to local file %s\n", filename);
            fclose(file);
            Close(clientfd);
            exit(1);
        }
    }

    fclose(file);
    Close(clientfd);
    exit(0);
}
