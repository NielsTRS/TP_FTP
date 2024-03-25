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
    char buf[BLOCK_SIZE];
    FILE *file;
    Response res;
    ssize_t bytes_read;

    file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        send_response(&res, 404, "File not found", NULL);
        Rio_writen(connfd, &res, sizeof(res));
        return;
    }

    printf("Sending content of %s\n", filename);
    send_response(&res, 200, "File found", filename);
    Rio_writen(connfd, &res, sizeof(res));

    for (int i = 0; i <= res.block_number; i++) {
        bytes_read = fread(buf, 1, BLOCK_SIZE, file);
        if (bytes_read < BLOCK_SIZE) {
            if (feof(file)) {
                printf("End of file reached.\n");
            } else if (ferror(file)) {
                fprintf(stderr, "Error reading from file, stopped at block %d\n", i);
                break;
            }
        }
        Rio_writen(connfd, buf, bytes_read);
    }

    Fclose(file);
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    clientlen = (socklen_t)
            sizeof(clientaddr);

    Request req;

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

                get_request(connfd, &req, req.filename);
                printf("Received request for : %s\n", req.filename);
                send_file(connfd, req.filename);

                Close(connfd);
            }
        }
    }

    while (1);

}