//
// Created by Varun Bankar on 21/03/22.
// This code is part of lab assignment 7 of CS F303
// This code has companion client code 'tcp_client.c'
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#define TRUE 1
#define FALSE 0
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 4

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

int main(int argc, char *argv[]) {

    // port
    long server_port = strtol(argv[1], NULL, 0);

    // socket descriptors
    int connection_sockets[MAX_CLIENTS], connection_socket, server_socket, max_socket_descriptor, activity;

    // addresses
    struct sockaddr_in address, server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    // bookkeeping
    size_t received_bytes, read_bytes, sent_bytes;
    int status;
    int flag;

    // set of socket descriptors
    fd_set socket_set;

    // message buffer
    char buffer[BUFFER_SIZE];

    // options
    int opt = TRUE;
    char *shutdown_signal = "exit";
    char *connection_full = "ERROR: Connection full.";
    char *ok = "OK";

    // initialise all connection sockets to 0
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        connection_sockets[i] = 0;
    }

    // create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // set server socket to allow multiple connections
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const void *) &opt, sizeof(int));

    // define server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(server_port);

    // bind server socket to server port
    status = bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (status == -1) {
        perror("ERROR");
        exit(-1);
    }

    //try to specify maximum of 3 pending connections for the master socket
    listen(server_socket, MAX_CLIENTS + 1);
    printf("INFO: Server started on port %ld.\n", server_port);

    while (TRUE) {

        // clear socket set
        FD_ZERO(&socket_set);

        // add server socket to set
        FD_SET(server_socket, &socket_set);
        max_socket_descriptor = server_socket;

        // add connection sockets to set
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            connection_socket = connection_sockets[i];

            if (connection_socket > 0) {
                FD_SET(connection_socket, &socket_set);
            }

            if (connection_socket > max_socket_descriptor) {
                max_socket_descriptor = connection_socket;
            }
        }

        // wait for an activity on one of the sockets
        activity = select(max_socket_descriptor + 1, &socket_set, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            perror("ERROR");
            exit(-1);
        }

        // activity at server socket
        if (FD_ISSET(server_socket, &socket_set)) {
            // new connection
            connection_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);

            printf("INFO: New connection established (%d, %s:%d).\n", connection_socket,
                   inet_ntoa(client_address.sin_addr),
                   ntohs(client_address.sin_port));

            // add new connection socket to array of sockets
            flag = FALSE;
            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if (connection_sockets[i] == 0) {
                    connection_sockets[i] = connection_socket;
                    flag = TRUE;
                    break;
                }
            }

            if (flag == FALSE) {
                // connection full
                flush_buffer(buffer);
                memcpy(buffer, connection_full, strlen(connection_full));
                send(connection_socket, buffer, strlen(buffer), 0);
                close(connection_socket);
                printf("INFO: Disconnected (%d, %s:%d) - Connection full.\n", connection_socket,
                       inet_ntoa(client_address.sin_addr),
                       ntohs(client_address.sin_port));
            } else {
                // connection successful
                flush_buffer(buffer);
                memcpy(buffer, ok, strlen(ok));
                send(connection_socket, buffer, strlen(buffer), 0);
            }

            continue;
        }

        // activity at connection sockets
        for (int i = 0; i < MAX_CLIENTS; ++i) {

            connection_socket = connection_sockets[i];

            if (FD_ISSET(connection_socket, &socket_set)) {
                // receive data from client
                flush_buffer(buffer);
                received_bytes = recv(connection_socket, buffer, sizeof(buffer), 0);
                getpeername(connection_socket, (struct sockaddr *) &client_address, &client_address_len);

                // handle exit signal
                if (memcmp(buffer, shutdown_signal, strlen(shutdown_signal)) == 0) {
                    printf("INFO: Disconnecting (%s:%d) -> %s\n", inet_ntoa(client_address.sin_addr),
                           ntohs(client_address.sin_port),
                           buffer);
                    close(connection_socket);
                    connection_sockets[i] = 0;
                    continue;
                }

                // output reversed message
                reverse_string(buffer);
                printf("INFO: (%s:%d) -> %s\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port),
                       buffer);

            }

        }

        // read from stdin and send to all connected clients
        flush_buffer(buffer);
        printf("> ");
        scanf("%s", buffer);
        for (int j = 0; j < MAX_CLIENTS; ++j) {
            if (connection_sockets[j] > 0) {
                send(connection_sockets[j], buffer, strlen(buffer), 0);
            }
        }

    }

    // close server socket
    close(server_socket);

    return 0;
}
