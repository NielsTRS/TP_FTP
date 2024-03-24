/*
 * client.c - A client for the concurrent server with pool of processes
 */

#include "csapp.h"
#include "protocol.h"

int main(int argc, char **argv) {
    int clientfd;
    char *host, file_buf[BLOCK_SIZE];
    Response res;
    Request req;
    FILE *file;
    ssize_t bytes_read;
    long bytes_to_read;
    clock_t start, end;
    float total_time;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <filename> \n", argv[0]);
        exit(0);
    }

    host = argv[1];

    clientfd = Open_clientfd(host, PORT);

    printf("client connected to server OS\n");

    send_request(clientfd, &req, argv[2]); // Send request to server

    if (Rio_readn(clientfd, &res, sizeof(res)) > 0) { // Get response from server
        get_response(&res, &res.status, res.message, &res.file_size);
        bytes_to_read = res.file_size;
        if (res.status == 200) {
            printf("%s\n", res.message);
            file = Fopen(req.filename, "wb"); // Open or create a local file for writing in binary mode
            if (file != NULL) {
                start = clock();

                while (bytes_to_read > 0 && (bytes_read = Rio_readn(clientfd, file_buf, BLOCK_SIZE)) > 0) {
                    bytes_to_read -= bytes_read;
                    if (fwrite(file_buf, 1, bytes_read, file) != bytes_read) {
                        fprintf(stderr, "Error writing to local file %s\n", req.filename);
                        Fclose(file);
                        Close(clientfd);
                        exit(1);
                    }
                }

                end = clock();

                total_time = (end - start) * 1e-6;

                printf("File %s received and saved\n", req.filename);
                printf("%zd bytes received in %f seconds : (%f Kbytes / s) \n", res.file_size, total_time,
                       (res.file_size / total_time) / 1024);
                Fclose(file);
            } else {
                fprintf(stderr, "Error opening local file %s\n", req.filename);
            }
        } else {
            printf("%s\n", res.message);
        }
    }

    Close(clientfd);
    exit(0);
}
