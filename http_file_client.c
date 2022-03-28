#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>


int main(int argc, char *argv[]) {

    // file url
    char *f_url = argv[1];

    // declarations
    int i, j, https, status, content_length, client_socket;
    size_t f_url_length;
    char domain[BUFSIZ], path[BUFSIZ], filename[BUFSIZ], buffer[BUFSIZ], ip_address[128], request_buffer[BUFSIZ], response_buffer[BUFSIZ], *ptr;
    char *request = "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n";
    ssize_t bytes_sent, bytes_received, bytes_written;

    // initializations
    f_url_length = strlen(f_url);
    https = 0;
    bytes_written = 0;

    // extract filename
    for (i = (int) f_url_length - 1; i >= 0; i--) {
        if (f_url[i] == '/') {
            strcpy(filename, f_url + i + 1);
            break;
        }
    }

    // https check
    if (f_url[4] == 's') {
        https = 1;
    }

    // extract domain
    for (j = 7 + https; j < f_url_length; j++) {
        if (f_url[j] == '/') {
            strncpy(domain, f_url + 7 + https, j - 7 - https);
            break;
        }
    }

    // extract path
    strcpy(path, f_url + j + 1);

    printf("INFO: \nHTTPS: %d, \nDomain: %s, \nFile Path: %s, \nFile Name: %s\n", https, domain, path, filename);

    struct hostent *he;
    struct in_addr **address_list;

    he = gethostbyname(domain);
    if (he == NULL) {
        perror("ERROR");
        exit(1);
    }
    address_list = (struct in_addr **) he->h_addr_list;

    // extract ip_address address
    for (i = 0; address_list[i] != NULL; i++) {
        strcpy(ip_address, inet_ntoa(*address_list[i]));
        break;
    }

    // create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("ERROR");
        exit(1);
    }

    puts("INFO: Socket created successfully.");

    // server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip_address);
    server_address.sin_port = htons(80);

    // connect to server
    status = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (status != 0) {
        perror("ERROR");
        exit(1);
    }

    puts("INFO: Connected to server.");

    // add request_buffer to request_buffer buffer
    snprintf(request_buffer, sizeof(request_buffer), request, path, domain);

    // send request to server
    bytes_sent = send(client_socket, request_buffer, strlen(request_buffer), 0);
    if (bytes_sent < 0) {
        perror("ERROR");
        exit(1);
    }

    printf("INFO: Request send (%lu bytes):\n%s", strlen(request_buffer), request_buffer);

    // remove file from current directory
    remove(filename);
    FILE *fd = fopen(filename, "wb");

    // save response to response buffer
    ptr = response_buffer + 4;
    bytes_received = recv(client_socket, ptr, 1, 0);
    while (bytes_received) {
        if (bytes_received == -1) {
            perror("ERROR");
            exit(1);
        }
        if ((ptr[-3] == '\r') && (ptr[-2] == '\n') && (ptr[-1] == '\r') && (*ptr == '\n')) {
            break;
        }
        ptr++;
        bytes_received = recv(client_socket, ptr, 1, 0);
    }

    // reset pointer
    *ptr = 0;
    ptr = response_buffer + 4;

    // find content length
    ptr = strstr(ptr, "Content-Length:");
    if (ptr) {
        sscanf(ptr, "%*s %d", &content_length);
    } else {
        content_length = -1;
    }

    printf("INFO: Incoming content length: %d bytes.\n", content_length);

    // receive file
    bytes_received = recv(client_socket, buffer, 1024, 0);
    while (bytes_received) {
        if (bytes_received == -1) {
            perror("ERROR");
            exit(1);
        }

        // write to disk
        fwrite(buffer, 1, bytes_received, fd);
        bytes_written += bytes_received;

        // all downloaded
        if (bytes_written == content_length) {
            break;
        }

        // receive next segment of file
        bytes_received = recv(client_socket, buffer, 1024, 0);
    }

    printf("INFO: Finished downloading file: %s.\n", filename);
    puts("Done.");

    // close descriptors
    fclose(fd);
    close(client_socket);

    return 0;
}