#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#define MAX_USERNAME_LEN 256
#define MAX_CLIENTS 3
#define BUFFER_SIZE 1024
#define TRUE 1
#define FALSE 0

struct client {
    int socket_id;
    char username[MAX_USERNAME_LEN];
    struct sockaddr_in address;
};

// flush buffer - set every byte to null char
void flush_buffer(char *buffer) {
    memset(buffer, '\0', BUFFER_SIZE);
}

int main(int argc, char *argv[]) {

    // port
    long server_port = strtol(argv[1], NULL, 0);

    // socket descriptors
    int server_socket, connection_socket;

    // addresses
    struct sockaddr_in server_address;

    // data structure to store clients
    struct client clients[MAX_CLIENTS];

    // bookkeeping
    size_t received_bytes, read_bytes, sent_bytes;
    int status;
    int flag;
    int client_count = 0;
    socklen_t client_address_len = sizeof(clients[0].address);

    // message buffer
    char buffer[BUFFER_SIZE];

    // options
    int opt = TRUE;
    char *poll_signal = "POLL";
    char *none_signal = "NONE";
    char *exit_signal = "EXIT";
    char *list_signal = "LIST";
    char *send_signal = "SEND";

    // initialise all client sockets to 0
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clients[i].socket_id = 0;
        flush_buffer(clients[i].username);
    }

    // create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // TODO: set server socket to allow multiple connections
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

    // try to specify maximum of 3 pending connections for the master socket
    listen(server_socket, MAX_CLIENTS + 1);
    printf("INFO: Server started on port %ld.\n", server_port);

    // accept clients
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        connection_socket = accept(server_socket, (struct sockaddr *) &clients[i].address, &client_address_len);
        clients[i].socket_id = connection_socket;

        // receive username
        flush_buffer(buffer);
        received_bytes = recv(connection_socket, buffer, sizeof(buffer), 0);
        memcpy(clients[i].username, buffer, strlen(buffer));

        // increment client count
        client_count++;

        printf("INFO: New connection established (%d, %s:%d).\n",
               connection_socket,
               inet_ntoa(clients[i].address.sin_addr),
               ntohs(clients[i].address.sin_port)
        );
    }

    // start polling
    while (client_count) {

        // accept clients
        for (int i = 0; i < MAX_CLIENTS; ++i) {

            if (clients[i].socket_id == 0) continue;

            printf("INFO: Polling client (%d, %s, %s:%d).\n",
                   clients[i].socket_id,
                   clients[i].username,
                   inet_ntoa(clients[i].address.sin_addr),
                   ntohs(clients[i].address.sin_port)
            );

            // send POLL signal
            send(clients[i].socket_id, poll_signal, strlen(poll_signal), 0);

            // wait for reply
            flush_buffer(buffer);
            recv(clients[i].socket_id, buffer, sizeof(buffer), 0);

            if (memcmp(buffer, none_signal, strlen(none_signal)) == 0) {
                continue;
            }

            if (memcmp(buffer, exit_signal, strlen(exit_signal)) == 0) {
                clients[i].socket_id = 0;
                continue;
            }

            if (memcmp(buffer, exit_signal, strlen(exit_signal)) == 0) {
                clients[i].socket_id = 0;
                client_count--;
                continue;
            }

            if (memcmp(buffer, list_signal, strlen(list_signal)) == 0) {
                flush_buffer(buffer);
                for (int j = 0; j < MAX_CLIENTS; ++j) {
                    strcat(buffer, clients[j].username);
                    strcat(buffer, ", ");
                }
                send(clients[i].socket_id, buffer, strlen(buffer), 0);
                continue;
            }

            if (memcmp(buffer, send_signal, strlen(send_signal)) == 0) {
                // TODO
                continue;
            }

        }

    }


    // close server socket
    close(server_socket);

    return 0;
}

