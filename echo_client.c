//
// Check out 'echo_server.c' for server implementation for this client.
//

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 2048

void flush_buffer(char *buffer, int size) {
    for (int i = 0; i < size; ++i) {
        buffer[i] = '\0';
    }
}

int main(int argc, char const *argv[]) {

    char const *s_addr = argv[1];
    int server_port = atoi(argv[2]);

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
        perror("ERROR: Failed to connect to server.");
        exit(-1);
    }

    while (1) {
        char *buffer[BUFFER_SIZE];
        flush_buffer(buffer, BUFFER_SIZE);

        printf("Enter your message: ");
        scanf("%s", buffer);

        send(client_socket, buffer, strlen(buffer), 0);
        printf("INFO: Message sent to server.\n");

        if (strcmp(buffer, "exit") == 0) break;
        if (strcmp(buffer, "shutdown") == 0) break;

        flush_buffer(buffer, BUFFER_SIZE);
        ssize_t received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
        printf("INFO: Message received from server (%zd bytes):\n%s\n", received_bytes, buffer);
    }

    return 0;
}