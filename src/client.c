/*
 * client.c - A client for the concurrent server with pool of processes
 */

#include "csapp.h"
#include "protocol.h"

int main(int argc, char **argv) {
    int clientfd;
    char *host;
    char filename_buf[MAX_NAME_LEN];
    Response res;
    Request req;
    FILE *file;
    ssize_t bytes_read, total_bytes = 0;
    clock_t start, end;
    float total_time;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host> \n", argv[0]);
        exit(0);
    }

    host = argv[1];

    clientfd = Open_clientfd(host, PORT);

    printf("client connected to server OS\n");

    while (Fgets(filename_buf, MAX_NAME_LEN, stdin) != NULL) {
        if (strcmp(filename_buf, "bye\n") == 0) {
            break;
        }

        total_bytes = 0;
        filename_buf[strlen(filename_buf) - 1] = '\0';
        send_request(clientfd, &req, filename_buf); // Send request to server

        get_response(clientfd, &res, &res.status, &res.block_number, res.message, &res.file_size);
        if (res.status == 200) {
            printf("%s\n", res.message);
            file = Fopen(req.filename, "wb"); // Open or create a local file for writing in binary mode
            if (file != NULL) {
                start = clock();

                for (long i = 0; i < res.block_number; i++) {
                    Block block;
                    Rio_readn(clientfd, &block, sizeof(Block));
                    bytes_read = block.size;
                    total_bytes += bytes_read;
                    fwrite(block.buf, 1, bytes_read, file);
                }

                if (total_bytes != res.file_size) {
                    fprintf(stderr, "Error: received %zd bytes, expected %zd bytes\n", total_bytes, res.file_size);
                    Fclose(file);
                    break;
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
