#include "csapp.h"
#include "protocol.h"

#define MAX_SLAVES 5
#define PORT 2121
#define CONFIG_FILE "config.txt"

typedef struct {
    Slave slaves[MAX_SLAVES];
    int num_slaves;
    int next_slave;
} ServerState;


void add_slave(ServerState *state, char *ip, int port) {
    Slave *slaves = state->slaves;
    strcpy(slaves[state->num_slaves].ip, ip);
    slaves[state->num_slaves].port = htonl(port);
    state->num_slaves++;
}

void handle_client(ServerState *state, int connfd) {
    Slave *slave = &state->slaves[state->next_slave];
    state->next_slave = (state->next_slave + 1) % state->num_slaves;
    Rio_writen(connfd, slave, sizeof(Slave));
    printf("Redirecting client to server : %s:%d\n", slave->ip, htonl(slave->port));
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    FILE *config_file;
    char ip[16];
    int port;
    ServerState state;
    int slavefd;

    listenfd = Open_listenfd(PORT);

    config_file = fopen(CONFIG_FILE, "r");
    if (config_file == NULL) {
        fprintf(stderr, "Could not open config file\n");
        exit(1);
    }

    while (fscanf(config_file, "%s %d\n", ip, &port) != EOF && state.num_slaves < MAX_SLAVES) {
        slavefd = open_clientfd(ip, port);
        if(slavefd < 0) {
            fprintf(stderr, "Could not connect to slave %s:%d\n", ip, port);
        } else {
            printf("Slave %s:%d is online\n", ip, port);
            add_slave(&state, ip, port);
            Close(slavefd);
        }
    }
    fclose(config_file);

    if (state.num_slaves == MAX_SLAVES) {
        printf("Max number of slaves reached\n");
    }

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        printf("Client connected\n");
        handle_client(&state, connfd);
        Close(connfd);
    }

}