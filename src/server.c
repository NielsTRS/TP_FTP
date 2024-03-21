/*
 * server.c - A concurrent server with pool of processes
 */

#include "csapp.h"
#include "protocol.h"

#define MAX_NAME_LEN 256
#define NB_PROC 3
#define PORT 2121

pid_t pids[NB_PROC];

void sigint_handler(int sig) {
    for (int i = 0; i < NB_PROC; i++) {
        Kill(pids[i], SIGINT);
    }
    exit(0);
}

void send_file(int connfd, char *filename) {
    char buf[MAXLINE];
    FILE *file;
    Protocol protocol;
    ssize_t bytes_read;

    file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        protocol.status = 404;
        strcpy(protocol.message, "File not found");
        Rio_writen(connfd, &protocol, sizeof(protocol));
        return;
    }

    printf("Sending content of %s\n", filename);
    protocol.status = 200;
    strcpy(protocol.message, "File found");
    Rio_writen(connfd, &protocol, sizeof(protocol));

    if ((bytes_read = Fread(buf, 1, MAXLINE, file)) > 0) {
        Rio_writen(connfd, buf, bytes_read);
    }

    Fclose(file);
}

void get_filename(int connfd, char *filename) {
    if (Rio_readn(connfd, filename, MAX_NAME_LEN) != 0) {
        printf("Received request for : %s\n", filename);
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

    char filename[MAX_NAME_LEN];

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

                get_filename(connfd, filename);
                send_file(connfd, filename);

                Close(connfd);
            }
        }
    }

    while (1);

}