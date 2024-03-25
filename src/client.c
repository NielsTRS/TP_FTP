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
    ssize_t bytes_read, total_bytes = 0;
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
        get_response(&res, &res.status, &res.block_number, res.message, &res.file_size);
        if (res.status == 200) {
            printf("%s\n", res.message);
            file = Fopen(req.filename, "wb"); // Open or create a local file for writing in binary mode
            if (file != NULL) {
                start = clock();

                for (int i = 0; i <= res.block_number; i++) {
                    if ((bytes_read = Rio_readn(clientfd, file_buf, BLOCK_SIZE)) > 0) {
                        total_bytes += bytes_read;
                        if (fwrite(file_buf, 1, bytes_read, file) != bytes_read) {
                            fprintf(stderr, "Error writing to local file %s\n", req.filename);
                            Fclose(file);
                            Close(clientfd);
                            exit(1);
                        }
                    } else {
                        fprintf(stderr, "Error downloading file from server\n");
                        Fclose(file);
                        Close(clientfd);
                        exit(1);
                    }
                }

                if(total_bytes != res.file_size) {
                    fprintf(stderr, "Error: received %zd bytes, expected %zd bytes\n", total_bytes, res.file_size);
                    Fclose(file);
                    Close(clientfd);
                    exit(1);
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
