#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 4444

int main(int argc, char const *argv[]) {

    // message string
    char server_message[256] = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html;charset=UTF-8\r\n"
                               "\r\n"
                               "Hello World\r\n";

    // create server socket of type TCP
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // define server address - accept any ip address on port 4444
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // bind server socket to port 4444
    int binding_status = bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (binding_status == -1) {
        perror("ERROR: Failed to bind to given port.");
        exit(-1);
    }

    // listen for connections on port 4444
    listen(server_socket, 1);

    // accept a connection from client - establish a tunnel
    int connection_socket = accept(server_socket, NULL, NULL);
    printf("INFO: New connection established.\n");

    // request buffer
    char buffer[2048];
    recv(connection_socket, buffer, sizeof(buffer), 0);
    printf("\nRequest:\n%s\n", buffer);

    // send data to client
    send(connection_socket, server_message, sizeof(server_message), 0);
    printf("INFO: 'Hello World' message sent.\n");

    // close client connection
    close(connection_socket);
    printf("INFO: Connection closed.\n");

    // close server socket
    close(server_socket);

    return 0;

}

