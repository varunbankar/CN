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

// flush buffer - set every byte to null char
void flush_buffer(char *buffer) {
    memset(buffer, '\0', BUFFER_SIZE);
}

int main(int argc, char *argv[]) {

    // command line arguments destructured
    char const *username = argv[1];
    char const *s_addr = argv[2];
    long server_port = strtol(argv[3], NULL, 0);

    // memory buffer
    char buffer[BUFFER_SIZE];

    // bookkeeping
    int status;
    ssize_t received_bytes;

    // signals
    char *poll_signal = "POLL";
    char *none_signal = "NONE";
    char *exit_signal = "EXIT";
    char *list_signal = "LIST";
    char *send_signal = "SEND";

    // create server socket of type TCP
    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

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

    // send username
    flush_buffer(buffer);
    send(client_socket, username, strlen(username), 0);

    // wait to be polled
    while (TRUE) {
        // receive response from server
        flush_buffer(buffer);
        received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
        printf("SERVER MSG: %s\n", buffer);
        if (memcmp(buffer, poll_signal, strlen(poll_signal)) == 0) {
            flush_buffer(buffer);
            scanf("%s", buffer);
            send(client_socket, buffer, strlen(buffer), 0);

            if (memcmp(buffer, exit_signal, strlen(exit_signal)) == 0) {
                break;
            }

            printf("INFO: Request sent: %s\n", buffer);
        }
    }


    // close client socket
    close(client_socket);

    return 0;
}

