/*
 * server.c - A concurrent server with pool of processes
 */

#include "csapp.h"
#include "protocol.h"

#define NB_PROC 3

pid_t pids[NB_PROC];


void sigint_handler(int sig) {
    for (int i = 0; i < NB_PROC; i++) {
        Kill(pids[i], SIGINT);
    }
    exit(0);
}

void send_file(int connfd, char *filename) {
    FILE *file;
    Response res;
    ssize_t bytes_read;
    struct stat st;
    long block_number;

    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        send_response(connfd, &res, 404, "File not found", 0, 0);
        return;
    }

    fstat(fileno(file), &st);
    block_number = (st.st_size / BLOCK_SIZE) + 1;

    printf("Sending content of %s\n", filename);
    send_response(connfd, &res, 200, "File found", st.st_size, block_number);
    for (long i = 0; i < block_number; i++) {
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
        Rio_writen(connfd, &block, sizeof(Block));
    }
    printf("File sent\n");

    fclose(file);
}

void handle_request(int fd) {
    Request req;
    while (get_request(fd, &req, req.filename)) {
        printf("Received request for : %s\n", req.filename);
        send_file(fd, req.filename);
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

    Signal(SIGINT, sigint_handler);

    listenfd = Open_listenfd(PORT);

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