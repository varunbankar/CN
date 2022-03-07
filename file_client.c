//
// Created by Varun Bankar on 26/02/22.
// This code is part of lab assignment 6 of CS F303
// This code has companion client code 'file_server.c'
//

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 2048


int main(int argc, char const *argv[]) {

    // command line arguments destructured
    char const *s_addr = argv[1];
    int server_port = atoi(argv[2]);
    char const *filename = argv[3];

    // memory buffer
    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', sizeof(buffer));  // flush buffer

    // file pointer
    FILE *fptr;

    // create server socket of type TCP
    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // define server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    inet_pton(AF_INET, s_addr, &(server_address.sin_addr));

    int connect_status = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (connect_status == -1) {
        perror("ERROR");
        exit(-1);
    }

    send(client_socket, filename, strlen(filename), 0);
    printf("INFO: Request sent: %s\n", filename);

    ssize_t received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
    printf("INFO: Response received (%zd bytes):%s\n", received_bytes, buffer);

    fptr = fopen(filename, "w");
    if (fptr == NULL) {
        perror("ERROR");
        exit(-1);
    }

    fwrite(buffer, sizeof(char), received_bytes, fptr);
    printf("INFO: Contents written to file successfully.\n");

    close(client_socket);

    return 0;
}