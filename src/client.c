/*
 * client.c - A client for the concurrent server with pool of processes
 */

#include "csapp.h"
#include "protocol.h"

void receive_file(int fd, Response *res, Request *req) {
    FILE *file;
    Block block;
    ssize_t result;

    file = Fopen(req->filename, "wb"); // Open or create a local file for writing in binary mode
    if (file != NULL) {
        for (long i = 0; i < res->block_number; i++) {
            result = Rio_readn(fd, &block, sizeof(Block));
            if (result < sizeof(Block)) {
                fprintf(stderr, "Error reading from socket: only %zd out of %zd bytes read\n", result, sizeof(Block));
                Fclose(file);
                return;
            }
            result = fwrite(block.buf, 1, block.size, file);
            if (result < block.size) {
                fprintf(stderr, "Error writing to file: only %zd out of %zd bytes written\n", result, block.size);
                Fclose(file);
                return;
            }
        }

        printf("File %s received and saved\n", req->filename);
        Fclose(file);
    } else {
        fprintf(stderr, "Error opening local file %s\n", req->filename);
    }
}

int main(int argc, char **argv) {
    int clientfd;
    char *host;
    char user_input[MAX_NAME_LEN];
    Response res;
    Request req;
    clock_t start, end;
    float total_time;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <host> \n", argv[0]);
        exit(0);
    }

    host = argv[1];

    clientfd = Open_clientfd(host, PORT);

    printf("Client connected to server OS\n");

    while (Fgets(user_input, MAX_NAME_LEN, stdin) != NULL) {
        if (strcmp(user_input, "bye\n") == 0) {
            break;
        }

        user_input[strlen(user_input) - 1] = '\0';

        send_request(clientfd, &req, user_input); // Send request to server
        if (get_response(clientfd, &res, &res.status, &res.block_number, res.message, &res.file_size)) {
            printf("Received response from server\n");
            if (res.status == 200) {
                printf("%s\n", res.message);
                start = clock();
                receive_file(clientfd, &res, &req);
                end = clock();
                total_time = (end - start) * 1e-6;

                printf("%zd bytes received in %f seconds : (%f Kbytes / s) \n", res.file_size, total_time,
                       (res.file_size / total_time) / 1024);
            } else {
                printf("%s\n", res.message);
            }
        } else {
            fprintf(stderr, "Error receiving response from server\n");
            break;
        }
    }

    Close(clientfd);
    exit(0);
}
