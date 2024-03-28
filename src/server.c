/*
 * server.c - A concurrent server with pool of processes
 */

#include "csapp.h"
#include "protocol.h"

#define NB_PROC 10
#define FILE_DIRECTORY "files/"

pid_t pids[NB_PROC];

void sigint_handler(int sig) {
    for (int i = 0; i < NB_PROC; i++) {
        Kill(pids[i], SIGINT);
    }
    exit(0);
}

void send_file(int connfd, char *filename, long starting_block) {
    FILE *file;
    Response res;
    ssize_t bytes_read;
    struct stat st;
    long block_number;
    long file_size;
    char full_path[MAX_NAME_LEN];

    strcpy(full_path, FILE_DIRECTORY);
    strcat(full_path, filename);

    file = fopen(full_path, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        send_response(connfd, &res, 404, "File not found", 0, 0);
        return;
    }

    fstat(fileno(file), &st);
    block_number = (st.st_size / BLOCK_SIZE) + 1;
    file_size = st.st_size - (starting_block * BLOCK_SIZE);

    // Skip to the start block
    fseek(file, starting_block * BLOCK_SIZE, SEEK_SET);

    printf("Sending content of %s\n", filename);
    send_response(connfd, &res, 200, "File found", file_size, block_number);
    for (long i = starting_block; i < block_number; i++) {
        Block block;
        bytes_read = fread(block.buf, 1, BLOCK_SIZE, file);
        if (bytes_read < BLOCK_SIZE) {
            if (feof(file)) {
                printf("End of file reached\n");
            } else {
                fprintf(stderr, "Error reading from file: %s\n", strerror(errno));
                fclose(file);
                return;
            }
        }
        block.size = bytes_read;
        if (rio_writen(connfd, &block, sizeof(Block)) < 0) {
            fprintf(stderr, "Client disconnected during transfer\n");
            fclose(file);
            return;
        }
    }
    printf("File sent\n");

    fclose(file);
}

void handle_request(int fd) {
    Request req;
    Response res;
    while (get_request(fd, &req, req.user_input, &req.starting_block)) {
        if (strncmp(req.user_input, "get ", 4) == 0) {
            char* filename = req.user_input + 4;
            printf("Received request for %s starting at block %ld\n", req.user_input, req.starting_block);
            send_file(fd, filename, req.starting_block);
        } else {
            printf("Command not yet implemented\n");
            send_response(fd, &res, 200, "Command not yet implemented", 0, 0);
        }

    }
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    clientlen = (socklen_t)
            sizeof(clientaddr);
    struct stat st;
    int port;

    Signal(SIGINT, sigint_handler);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(0);
    }

    port = atoi(argv[1]);

    // make directory if it doesn't exist
    if (stat(FILE_DIRECTORY, &st) == -1) {
        mkdir(FILE_DIRECTORY, 0700);
    }

    listenfd = Open_listenfd(port);

    printf("Main server PID : %d\n", getpid());

    for (int i = 0; i < NB_PROC; i++) {
        if ((pids[i] = Fork()) == 0) { // Child process
            while (1) {
                connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);

                Getnameinfo((SA *) &clientaddr, clientlen,
                            client_hostname, MAX_NAME_LEN, 0, 0, 0);

                Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                          INET_ADDRSTRLEN);

                printf("server connected to %s (%s) using process %d\n", client_hostname,
                       client_ip_string, getpid());

                handle_request(connfd);

                printf("Client %s (%s) disconnected\n", client_hostname, client_ip_string);

                Close(connfd);
            }
        }
    }

    while (1);

}