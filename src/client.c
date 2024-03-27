/*
 * client.c - A client for the concurrent server with pool of processes
 */

#include "csapp.h"
#include "protocol.h"
#include <dirent.h>

#define EXTENSION ".part"
#define FILE_DIRECTORY "files/"
#define PORT 2121 // master server port

// Get the last valid block number received from the server
long get_last_received_block_number(char *filename) {
    long last_number = 0;
    long previous_last_number = 0;
    int next_char;
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return -1;
    }
    while (fscanf(file, "%ld", &last_number) == 1) {
        next_char = fgetc(file);
        if (next_char == '\n') {
            previous_last_number = last_number;
        }
    }

    fclose(file);
    return previous_last_number;
}

// file manager
void receive_file(int fd, Response *res, Request *req) {
    FILE *file;
    FILE *save_file;
    Block block;
    ssize_t result;

    char full_path[MAX_NAME_LEN];
    char full_path_save[MAX_NAME_LEN];

    strcpy(full_path, FILE_DIRECTORY);
    strcat(full_path, req->user_input);

    strcpy(full_path_save, full_path);
    strcat(full_path_save, EXTENSION);

    if (access(full_path, F_OK) != -1 && access(full_path_save, F_OK) != -1) {
        file = fopen(full_path, "rb+");
    } else {
        file = fopen(full_path, "wb");
    }

    save_file = fopen(full_path_save, "w"); // store the position of the received blocks
    if (file != NULL) {
        if (save_file != NULL) {
            long starting_block = ntohl(req->starting_block);
            fseek(file, starting_block * BLOCK_SIZE, SEEK_SET);
            for (long i = starting_block; i < res->block_number; i++) {
                result = Rio_readn(fd, &block, sizeof(Block));
                if (result < sizeof(Block)) {
                    fprintf(stderr, "Error reading from socket: only %zd out of %zd bytes read\n", result,
                            sizeof(Block));
                    fclose(file);
                    fclose(save_file);
                    return;
                }
                result = fwrite(block.buf, 1, block.size, file);
                if (result < block.size) {
                    fprintf(stderr, "Error writing to file: only %zd out of %zd bytes written\n", result, block.size);
                    fclose(file);
                    fclose(save_file);
                    return;
                }
                fprintf(save_file, "%ld\n", i);
            }

            printf("File %s received and saved\n", req->user_input);
            fclose(save_file);
            fclose(file);
            remove(full_path_save);
        } else {
            fprintf(stderr, "Error opening save file %s\n", full_path_save);
        }
    } else {
        fprintf(stderr, "Error opening local file %s\n", full_path);
    }
}

// Process request and response
void process(int fd, char *user_input, long starting_block, int type) {
    Response res;
    Request req;
    clock_t start, end;
    float total_time;

    send_request(fd, &req, user_input, starting_block, type); // Send request to server
    if (type == FILE_TYPE) {
        if (get_response(fd, &res, &res.status, &res.block_number, res.message, &res.file_size)) {
            printf("Received response from server\n");
            if (res.status == 200) {
                printf("%s\n", res.message);
                start = clock();
                receive_file(fd, &res, &req);
                end = clock();
                total_time = (end - start) * 1e-6;

                printf("%zd bytes received in %f seconds : (%f Kbytes / s) \n", res.file_size, total_time,
                       (res.file_size / total_time) / 1024);
            } else {
                printf("%s\n", res.message);
            }
        } else {
            fprintf(stderr, "Error receiving response from server\n");
            exit(0);
        }
    } else {
        printf("Command response : %s\n", res.message);
    }
}

// Check for incomplete files and resume download
void backup_part_files(int fd) {
    DIR *d;
    struct dirent *dir;
    char filename[MAX_NAME_LEN];
    long last_block;

    printf("Checking for incomplete files\n");

    d = opendir(FILE_DIRECTORY);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strstr(dir->d_name, EXTENSION) != NULL) {
                strcpy(filename, FILE_DIRECTORY);
                strcat(filename, dir->d_name);

                last_block = get_last_received_block_number(filename);
                printf("Found incomplete file %s with %ld blocks\n", filename, last_block);
                if (last_block != -1) {
                    filename[strlen(filename) - strlen(EXTENSION)] = '\0';
                    printf("Resuming download of : %s\n", filename);
                    process(fd, filename, last_block + 1, FILE_TYPE);
                }
            }
        }
        closedir(d);
    }
    printf("Check complete\n");
}

int is_command(char *user_input) {
    if (strcmp(user_input, "ls") == 0 || strcmp(user_input, "pwd") == 0 || strcmp(user_input, "cd") == 0) {
        return 1;
    } else {
        return 0;
    }
}

int main(int argc, char **argv) {
    int clientfd;
    char *host;
    char user_input[MAX_NAME_LEN];
    struct stat st;
    Slave slave;

    if (argc == 3) { // Direct connection to server
        host = argv[1];
        int port = atoi(argv[2]);
        clientfd = Open_clientfd(host, port);
        printf("Connected directly to server at %s:%d\n", host, port);
    } else if (argc == 2) { // Connection to master server
        host = argv[1];
        clientfd = Open_clientfd(host, PORT);
        printf("Connected to master\n");
        if (get_slave_data(clientfd, &slave, slave.ip, &slave.port) == 1) {
            Close(clientfd);
            printf("Connecting to slave server at %s:%d\n", slave.ip, slave.port);
            clientfd = Open_clientfd(slave.ip, slave.port);
        } else {
            Close(clientfd);
            fprintf(stderr, "Error receiving slave data from master\n");
            exit(0);
        }
    } else {
        fprintf(stderr, "Usage: %s <host> (Option : <port>)\n", argv[0]);
        exit(0);
    }

    // make directory if it doesn't exist
    if (stat(FILE_DIRECTORY, &st) == -1) {
        mkdir(FILE_DIRECTORY, 0700);
    }

    backup_part_files(clientfd);

    printf("\nEnter the name of the file you want to download or 'bye' to exit\n");
    printf("ftp > ");
    while (Fgets(user_input, MAX_NAME_LEN, stdin) != NULL) {
        user_input[strlen(user_input) - 1] = '\0';

        if (strcmp(user_input, "bye") == 0) {
            break;
        }

        if (is_command(user_input)) {
            process(clientfd, user_input, 0, COMMAND_TYPE);
        } else {
            process(clientfd, user_input, 0, FILE_TYPE);
        }

        printf("\nftp > ");
    }

    Close(clientfd);
    exit(0);
}
