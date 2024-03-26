/*
 * client.c - A client for the concurrent server with pool of processes
 */

#include "csapp.h"
#include "protocol.h"
#include <dirent.h>

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

void receive_file(int fd, Response *res, Request *req) {
    FILE *file;
    FILE *save_file;
    Block block;
    ssize_t result;
    char save_filename[MAX_NAME_LEN + 5];

    strcpy(save_filename, req->filename);
    strcat(save_filename, ".part");

    file = fopen(req->filename, "wb"); // Open or create a local file for writing in binary mode
    save_file = fopen(save_filename, "w"); // store the position of the received blocks
    if (file != NULL) {
        if (save_file != NULL) {
            for (long i = 0; i < res->block_number; i++) {
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

            printf("File %s received and saved\n", req->filename);
            fclose(save_file);
            fclose(file);
            remove(save_filename);
        } else {
            fprintf(stderr, "Error opening save file %s\n", save_filename);
        }
    } else {
        fprintf(stderr, "Error opening local file %s\n", req->filename);
    }
}

void handle(int fd, char *user_input) {
    Response res;
    Request req;
    clock_t start, end;
    float total_time;

    send_request(fd, &req, user_input); // Send request to server
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
}

void backup_part_files(int fd) {
    DIR *d;
    struct dirent *dir;
    printf("Checking for incomplete files\n");

    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strstr(dir->d_name, ".part") != NULL) {
                char *real_filename = strcpy(real_filename, dir->d_name);
                long last_block = get_last_received_block_number(dir->d_name);
                printf("Found incomplete file %s with %ld blocks\n", dir->d_name, last_block);
                if (last_block != -1) {
                    real_filename[strlen(dir->d_name) - strlen(".part")] = '\0';
                    printf("Resuming download of : %s\n", real_filename);
                    handle(fd, real_filename);
                }
            }
        }
        closedir(d);
    }
    printf("Check complete\n");
}

int main(int argc, char **argv) {
    int clientfd;
    char *host;
    char user_input[MAX_NAME_LEN];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <host> \n", argv[0]);
        exit(0);
    }

    host = argv[1];

    clientfd = Open_clientfd(host, PORT);

    printf("Client connected to server OS\n");

    backup_part_files(clientfd);

    printf("\nEnter the name of the file you want to download or 'bye' to exit\n");
    printf("ftp > ");
    while (Fgets(user_input, MAX_NAME_LEN, stdin) != NULL) {

        if (strcmp(user_input, "bye\n") == 0) {
            break;
        }

        user_input[strlen(user_input) - 1] = '\0';

        handle(clientfd, user_input);

        printf("\nftp > ");
    }

    Close(clientfd);
    exit(0);
}
