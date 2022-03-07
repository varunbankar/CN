//
// Created by Varun Bankar on 26/02/22.
// This code is part of lab assignment 6 of CS F303
// This code has companion client code 'file_client.c'
//

#include "stdio.h"
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define BUFFER_SIZE 2048
#define READ_BYTES 10

int main(int argc, char const *argv[]) {

    // server post command line argument
    int server_port = atoi(argv[1]);

    // memory buffer
    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', sizeof(buffer));  // flush buffer

    // shutdown signal
    char shutdown_signal[] = "shutdown";

    // file pointer
    FILE *fptr;

    // create server socket of type TCP
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // define server address - accept any ip address on server port
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // bind server socket to server port
    int binding_status = bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (binding_status == -1) {
        perror("ERROR");
        exit(-1);
    }

    // listen for connections on server port
    listen(server_socket, 1);
    printf("INFO: Started server on port %d.\n", server_port);

    while (1) {
        // accept a connection from client - establish a tunnel
        int connection_socket = accept(server_socket, NULL, NULL);
        printf("INFO: New connection established.\n");

        memset(buffer, '\0', sizeof(buffer));
        ssize_t received_bytes = recv(connection_socket, buffer, sizeof(buffer), 0);
        printf("INFO: Request received (%zd bytes): %s\n", received_bytes, buffer);

        if (memcmp(buffer, shutdown_signal, strlen(shutdown_signal)) == 0) {
            close(connection_socket);
            printf("INFO: Connection closed.\n");
            break;
        }

        fptr = fopen(buffer, "r");
        if (fptr == NULL) {
            perror("ERROR");
            send(connection_socket, buffer, 0, 0);  // send empty file
            close(connection_socket);
            printf("INFO: Connection closed.\n");
            continue;
        }

        memset(buffer, '\0', sizeof(buffer));  // flush buffer
        size_t read_bytes = fread(buffer, sizeof(char), READ_BYTES, fptr);
        if (read_bytes == 0) {
            send(connection_socket, buffer, 0, 0);  // Send empty file
            close(connection_socket);
            printf("INFO: Connection closed.\n");
            continue;
        }

        // send data to client
        send(connection_socket, buffer, read_bytes, 0);
        printf("INFO: Response sent (%zd bytes): %s\n", read_bytes, buffer);

        close(connection_socket);
        printf("INFO: Connection closed.\n");
    }

    // close server socket
    close(server_socket);
    printf("INFO: Server shutdown.\n");

    return 0;
}
