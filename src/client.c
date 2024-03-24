/*
 * client.c - A client for the concurrent server with pool of processes
 */

#include "csapp.h"
#include "protocol.h"

int main(int argc, char **argv) {
    int clientfd;
    char *host, buf[BLOCK_SIZE], filename[MAX_NAME_LEN];
    FILE *file;
    ssize_t bytes_read, total_bytes_read = 0;
    Protocol protocol;
    clock_t start, end;
    float total_time;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <filename> \n", argv[0]);
        exit(0);
    }

    host = argv[1];
    strncpy(filename, argv[2], strlen(argv[2]) + 1); // Copy filename to buffer

    clientfd = Open_clientfd(host, PORT);

    printf("client connected to server OS\n");

    Rio_writen(clientfd, filename, MAX_NAME_LEN); // write to server

    if (Rio_readn(clientfd, &protocol, sizeof(protocol)) > 0) { // Get response from server
        get_response(&protocol, &protocol.status, protocol.message);
        if (protocol.status == 404) {
            printf("%s\n", protocol.message);
            Close(clientfd);
            exit(0);
        } else if (protocol.status == 200) {
            printf("%s\n", protocol.message);
        }
    }

    file = Fopen(filename, "wb"); // Open or create a local file for writing in binary mode
    if (file == NULL) {
        fprintf(stderr, "Error opening local file %s\n", filename);
        Close(clientfd);
        exit(1);
    }

    start = clock();

    while ((bytes_read = Rio_readn(clientfd, buf, BLOCK_SIZE)) > 0) {
        total_bytes_read += bytes_read;
        if (fwrite(buf, 1, bytes_read, file) != bytes_read) {
            fprintf(stderr, "Error writing to local file %s\n", filename);
            Fclose(file);
            Close(clientfd);
            exit(1);
        }
    }

    end = clock();

    total_time = (end - start) * 1e-6;

    printf("File %s received and saved\n", filename);
    printf("%zd bytes received in %f seconds : (%f Kbytes / s) \n", total_bytes_read, total_time,
           (total_bytes_read / total_time) / 1024);
    Fclose(file);


    Close(clientfd);
    exit(0);
}
