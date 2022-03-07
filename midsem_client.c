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

    char campus_id[32], y[32], name[32];
    memset(campus_id, '\0', sizeof(campus_id));  // flush buffer
    memset(y, '\0', sizeof(y));  // flush buffer
    memset(name, '\0', sizeof(name));  // flush buffer

    // command line arguments destructured
    char const *s_addr = argv[1];
    int server_port = atoi(argv[2]);
    char const *filename = argv[3];

    // memory buffer
    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', sizeof(buffer));  // flush buffer

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

    // Scan ID
    scanf("%s", buffer);

    send(client_socket, buffer, strlen(buffer), 0);
    printf("INFO: Request sent: %s\n", filename);

    strcpy(campus_id, buffer);

    ssize_t received_bytes;

    memset(buffer, '\0', sizeof(buffer));  // flush buffer
    received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
    printf("INFO: Response received (%zd bytes):%s\n", received_bytes, buffer);

    strcpy(y, buffer);

    memset(buffer, '\0', sizeof(buffer));  // flush buffer

    int flag = 0;
    while (flag == 0) {

        scanf("%s", buffer);
        strcpy(name, buffer);

        send(client_socket, buffer, strlen(buffer), 0);
        printf("INFO: Request sent: %s\n", filename);

        memset(buffer, '\0', sizeof(buffer));  // flush buffer
        received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
        printf("INFO: Response received (%zd bytes):%s\n", received_bytes, buffer);

        if (strcmp(name, buffer) == 0) flag = 1;

    }

    printf("%s, %s, %s", campus_id, y, name);


    close(client_socket);

    return 0;
}