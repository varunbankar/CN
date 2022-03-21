//
// Created by Varun Bankar on 21/03/22.
// This code is part of lab assignment 7 of CS F303
// This code has companion client code 'tcp_server.c'
//

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define TRUE 1
#define BUFFER_SIZE 1024

// flush buffer - set every byte to null char
void flush_buffer(char *buffer) {
    memset(buffer, '\0', BUFFER_SIZE);
}

// reverse string
void reverse_string(char *s) {
    size_t start = 0, end = strlen(s) - 1;
    char temp;
    while (start < end) {
        temp = s[start];
        s[start] = s[end];
        s[end] = temp;
        start++;
        end--;
    }
}

int main(int argc, char const *argv[]) {

    // command line arguments destructured
    char const *s_addr = argv[1];
    long server_port = strtol(argv[2], NULL, 0);

    // memory buffer
    char buffer[BUFFER_SIZE];

    // bookkeeping
    int status;
    ssize_t received_bytes;

    // create server socket of type TCP
    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // options
    char *exit_signal = "exit";

    // define server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    inet_pton(AF_INET, s_addr, &(server_address.sin_addr));

    // connect to server
    status = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (status == -1) {
        perror("ERROR");
        exit(-1);
    }

    while (TRUE) {
        // read input from stdin and send to server
        flush_buffer(buffer);
        printf("> ");
        scanf("%s", buffer);
        send(client_socket, buffer, strlen(buffer), 0);
        printf("INFO: Request sent: %s\n", buffer);

        // exit signal
        if (memcmp(buffer, exit_signal, strlen(exit_signal)) == 0) {
            printf("INFO: Exiting...\n");
            break;
        }

        // receive response from server
        flush_buffer(buffer);
        received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
        printf("INFO: Response received (%zd bytes):%s\n", received_bytes, buffer);

        // output reverse response
        reverse_string(buffer);
        printf("INFO: Reversed: %s\n", buffer);
    }

    // close client socket
    close(client_socket);

    return 0;
}