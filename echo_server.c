//
// Check out 'echo_client.c' for client implementation for this server.
//

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 4445
#define BUFFER_SIZE 2048

void flush_buffer(char *buffer, int size) {
    for (int i = 0; i < size; ++i) {
        buffer[i] = '\0';
    }
}

int main(int argc, char const *argv[]) {

    // create server socket of type TCP
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // define server address - accept any ip address on port 4445
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

    // listen for connections on port 4445
    listen(server_socket, 1);
    printf("Started server on port %d\n", PORT);

    while (1) {
        // accept a connection from client - establish a tunnel
        int connection_socket = accept(server_socket, NULL, NULL);
        printf("INFO: New connection established.\n");

        int flag = 0;
        while (1) {
            // request buffer
            char buffer[BUFFER_SIZE];
            flush_buffer(buffer, BUFFER_SIZE);
            ssize_t received_bytes = 0;
            received_bytes = recv(connection_socket, buffer, sizeof(buffer), 0);
            printf("\nRequest (%zd bytes):\n%s\n", received_bytes, buffer);

            if (strcmp(buffer, "exit") == 0) {
                // close client connection
                close(connection_socket);
                printf("INFO: Connection closed.\n");
                break;
            } else if (strcmp(buffer, "shutdown") == 0) {
                // close client connection
                close(connection_socket);
                printf("INFO: Connection closed.\n");
                flag = 1;
                break;
            } else {
                // send data to client
                send(connection_socket, buffer, received_bytes, 0);
                printf("INFO: Response sent.\n");
            }
        }
        // break when shutdown command is received
        if (flag == 1) {
            break;
        }
    }

    // close server socket
    close(server_socket);
    printf("INFO: Server shutdown.\n");

    return 0;

}

