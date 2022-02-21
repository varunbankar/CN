//
// Code as part of lab assignment 5 of CS F303
// Author: Varun Bankar 2018B2A70295G
// Filename: client.c
//

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define SERVER_PORT 4444
#define BUFFER_SIZE 2048

void flush_buffer(char *buffer, int size) {
    for (int i = 0; i < size; ++i) {
        buffer[i] = '\0';
    }
}

int main(int argc, char const *argv[]) {

    // message buffer
    char buffer[BUFFER_SIZE];
    flush_buffer(buffer, sizeof(buffer));

    int server_port = SERVER_PORT;

    // create server socket of type TCP
    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // define server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // connect to server
    int connect_status = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (connect_status == -1) {
        perror("ERROR: Failed to connect to server.");
        exit(-1);
    }

    // first message from server
    recv(client_socket, buffer, sizeof(buffer), 0);
    printf("%s", buffer);

    // send name
    flush_buffer(buffer, sizeof(buffer));
    scanf("%s", buffer);
    send(client_socket, buffer, strlen(buffer), 0);

    // receive response code
    flush_buffer(buffer, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);

    // close connection on error
    if (strcmp(buffer, "500 ERROR") == 0) {
        flush_buffer(buffer, sizeof(buffer));
        strcpy(buffer, ".\n");
        send(client_socket, buffer, strlen(buffer), 0);
        flush_buffer(buffer, sizeof(buffer));
        recv(client_socket, buffer, sizeof(buffer), 0);
        exit(-1);
    }

    // read input and sent to server
    while (1) {
        flush_buffer(buffer, sizeof(buffer));
        fgets(buffer, sizeof(buffer), stdin);
        // scanf("%[^\n]%*c", buffer);
        send(client_socket, buffer, strlen(buffer), 0);
        if (strcmp(buffer, ".\n") == 0) {
            // receive response code
            flush_buffer(buffer, sizeof(buffer));
            recv(client_socket, buffer, sizeof(buffer), 0);
            printf("%s", buffer);
            break;
        }
    }

    return 0;
}