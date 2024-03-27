#include "csapp.h"
#include "protocol.h"

#define MAX_SLAVES 5
#define PORT 2121
#define CONFIG_FILE "config.txt"
#define RELOAD_INTERVAL 10

typedef struct {
    Slave slaves[MAX_SLAVES];
    int num_slaves;
    int next_slave;
} ServerState;

// Declare state as a global variable
ServerState state;

void add_slave(char *ip, int port) {
    Slave *slaves = state.slaves;
    strcpy(slaves[state.num_slaves].ip, ip);
    slaves[state.num_slaves].port = htonl(port);
    state.num_slaves++;
}

void handle_client(int connfd) {
    Slave *slave = &state.slaves[state.next_slave];
    state.next_slave = (state.next_slave + 1) % state.num_slaves;
    Rio_writen(connfd, slave, sizeof(Slave));
    printf("Redirecting client to server : %s:%d\n", slave->ip, htonl(slave->port));
}

void reload_config(int sig) {
    FILE *config_file;
    char ip[16];
    int port;

    config_file = fopen(CONFIG_FILE, "r");
    if (config_file == NULL) {
        fprintf(stderr, "Could not open config file\n");
        exit(1);
    }

    state.num_slaves = 0; // Reset the number of slaves

    while (fscanf(config_file, "%s %d\n", ip, &port) != EOF && state.num_slaves < MAX_SLAVES) {
        int slavefd = open_clientfd(ip, port);
        if(slavefd < 0) {
            fprintf(stderr, "Could not connect to slave %s:%d\n", ip, port);
        } else {
            printf("Slave %s:%d is online\n", ip, port);
            add_slave(ip, port);
            Close(slavefd);
        }
    }
    fclose(config_file);

    if (state.num_slaves == MAX_SLAVES) {
        printf("Max number of slaves reached\n");
    }

    // Reset the alarm
    alarm(RELOAD_INTERVAL);
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    listenfd = Open_listenfd(PORT);

    signal(SIGALRM, reload_config);

    reload_config(0);

    // Set the initial alarm
    alarm(RELOAD_INTERVAL);

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        printf("Client connected\n");
        handle_client(connfd);
        Close(connfd);
    }

}