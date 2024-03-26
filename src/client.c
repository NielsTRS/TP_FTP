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
    if(file == NULL) {
        return previous_last_number;
    }
    while (fscanf(file, "%ld", &last_number) == 1) {
        next_char = fgetc(file);
        if(next_char == '\n') {
            previous_last_number = last_number;
        }
    }

    fclose(file);
    printf("Last block received: %ld\n", previous_last_number);
    return previous_last_number;
}

void backup_part_files(){
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if(strstr(dir->d_name, ".part") != NULL){
                printf("%s\n", dir->d_name);
                get_last_received_block_number(dir->d_name);
            }
        }
        closedir(d);
    }
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

int main(int argc, char **argv) {
    int clientfd;
    char *host;
    char user_input[MAX_NAME_LEN];
    Response res;
    Request req;
    clock_t start, end;
    float total_time;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <host> \n", argv[0]);
        exit(0);
    }

    host = argv[1];

    clientfd = Open_clientfd(host, PORT);

    printf("Client connected to server OS\n");

    printf("Checking for incomplete files\n");
    backup_part_files();

    printf("\nEnter the name of the file you want to download or 'bye' to exit\n");
    printf("ftp > ");
    while (Fgets(user_input, MAX_NAME_LEN, stdin) != NULL) {

        if (strcmp(user_input, "bye\n") == 0) {
            break;
        }

        user_input[strlen(user_input) - 1] = '\0';

        send_request(clientfd, &req, user_input); // Send request to server
        if (get_response(clientfd, &res, &res.status, &res.block_number, res.message, &res.file_size)) {
            printf("Received response from server\n");
            if (res.status == 200) {
                printf("%s\n", res.message);
                start = clock();
                receive_file(clientfd, &res, &req);
                end = clock();
                total_time = (end - start) * 1e-6;

                printf("%zd bytes received in %f seconds : (%f Kbytes / s) \n", res.file_size, total_time,
                       (res.file_size / total_time) / 1024);
            } else {
                printf("%s\n", res.message);
            }
        } else {
            fprintf(stderr, "Error receiving response from server\n");
            break;
        }
        printf("\nftp > ");
    }

    Close(clientfd);
    exit(0);
}
