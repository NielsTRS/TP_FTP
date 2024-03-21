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
    char error_code[4];
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

    bytes_read = Rio_readn(clientfd, error_code, 4); // Read error code
    if (bytes_read > 0) {
        error_code[3] = '\0';
        if (strcmp(error_code, "404") == 0) {
            printf("Received error code from server: File not found\n");
            Close(clientfd);
            exit(0);
        } else if (strcmp(error_code, "200") == 0) {
            printf("File found, starting to receive file content\n");
        } else {
            printf("Received unknown error code from server: %s\n", error_code);
            Close(clientfd);
            exit(0);
        }
    }

    if ((bytes_read = Rio_readn(clientfd, buf, MAXLINE)) > 0) { // Read from server
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

    Close(clientfd);
    exit(0);
}
