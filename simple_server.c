//
// Code as part of lab assignment 5 of CS F303
// Author: Varun Bankar 2018B2A70295G
// Filename: server.c
//

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 4444
#define BUFFER_SIZE 2048

void flush_buffer(char *buffer, int size) {
    for (int i = 0; i < size; ++i) {
        buffer[i] = '\0';
    }
}

int main(int argc, char const *argv[]) {

    // message buffer
    char buffer[BUFFER_SIZE];
    char name[BUFFER_SIZE];
    flush_buffer(buffer, sizeof(buffer));

    // create server socket of type TCP
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // define server address - accept any ip address on port 4444
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // bind server socket to port 4445
    int binding_status = bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (binding_status == -1) {
        perror("ERROR: Failed to bind to given port.");
        exit(-1);
    }

    // listen for connections on port 4444
    listen(server_socket, 1);

    while (1) {
        // accept a connection from client - establish a tunnel
        int connection_socket = accept(server_socket, NULL, NULL);

        // ask for name
        flush_buffer(buffer, sizeof(buffer));
        strcpy(buffer, "Hello, what is your name?\n");
        send(connection_socket, buffer, strlen(buffer), 0);
        printf("%s", buffer);

        // receive name
        flush_buffer(buffer, sizeof(buffer));
        recv(connection_socket, buffer, sizeof(buffer), 0);
        printf("%s", buffer);

        // store name
        strcpy(name, buffer);

        // response status based on name
        if (name[0] >= 65 && name[0] <= 90) {
            flush_buffer(buffer, sizeof(buffer));
            strcpy(buffer, "200 OK");
            send(connection_socket, buffer, strlen(buffer), 0);
        } else {
            flush_buffer(buffer, sizeof(buffer));
            strcpy(buffer, "500 ERROR");
            send(connection_socket, buffer, strlen(buffer), 0);
        }

        // keep receiving messages until '.' is received
        while (1) {
            flush_buffer(buffer, sizeof(buffer));
            recv(connection_socket, buffer, sizeof(buffer), 0);
            printf("%s", buffer);

            if (strcmp(buffer, ".\n") == 0) {
                // send thank you message and close client connection
                flush_buffer(buffer, sizeof(buffer));
                strcpy(buffer, "Thank you, ");
                strcat(buffer, name);
                send(connection_socket, buffer, strlen(buffer), 0);
                printf("%s", buffer);
                close(connection_socket);
                break;
            }

        }
        // break
        break;
    }

    // close server socket
    close(server_socket);

    return 0;

}
