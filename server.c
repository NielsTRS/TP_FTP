/*
 * server.c - A concurrent server with pool of processes
 */

#include "csapp.h"

#define MAX_NAME_LEN 256
#define NB_PROC 3
#define PORT 2121

pid_t pids[NPROC];

void sigint_handler(int sig) {
    for (int i = 0; i < NPROC; i++) {
        Kill(pids[i], SIGINT);
    }
    exit(0);
}


void echo(int connfd);

/*
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
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

    printf("PID du veuilleur : %d\n", getpid());

    for (int i = 0; i < NPROC; i++) {
        if ((pids[i] = Fork()) == 0) { // Child process
            // le fils le plus rapide prendre en charge la connexion
            while (1) {
                connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);

                /* determine the name of the client */
                Getnameinfo((SA *) &clientaddr, clientlen,
                            client_hostname, MAX_NAME_LEN, 0, 0, 0);

                /* determine the textual representation of the client's IP address */
                Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                          INET_ADDRSTRLEN);

                printf("server connected to %s (%s) using process %d\n", client_hostname,
                       client_ip_string, getpid());

                echo(connfd);
                Close(connfd);
            }
        }
    }


    while (1);

}

