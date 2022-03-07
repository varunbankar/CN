//
// Created by Varun Bankar on 07/03/22.
// This code is part of lab mid sem of CS F303
// This code has companion client code 'midesem_client.c'
//

#include "stdio.h"
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define BUFFER_SIZE 2048
#define READ_BYTES 10

int is_lower(char *name) {
    printf("Name: %s (%d)\n", name, strlen(name));
    for (int i = 0; i < strlen(name); ++i) {
        if (name[i] >= 'A' && name[i] <= 'Z') {
            return 0;
        }
    }
    return 1;
}

int solve_math_equation(char *math_equation) {
    char operand1[32];
    char operand2[32];
    char operator;

    memset(operand1, '\0', sizeof(operand1));  // flush buffer
    memset(operand2, '\0', sizeof(operand2));  // flush buffer

    int math_equation_size = (int) strlen(math_equation);

    for (int i = 0; i < math_equation_size; ++i) {
        if (math_equation[i] >= '0' && math_equation[i] <= '9') {
            operand1[i] = math_equation[i];
        } else {
            operator = math_equation[i];
        }
    }

    int opc2 = 0;
    for (int i = (int) strlen(operand1); i < math_equation_size; ++i) {
        if (math_equation[i] >= '0' && math_equation[i] <= '9') {
            operand2[opc2++] = math_equation[i];
        }
    }

    printf("Operand1: %s\n", operand1);
    printf("Operand2: %s\n", operand2);
    printf("Operator: %c\n", operator);

    int op1 = atoi(operand1);
    int op2 = atoi(operand2);

    int result;

    switch (operator) {
        case '+':
            result = op1 + op2;
            break;
        case '-':
            result = op1 - op2;
            break;
        case '*':
            result = op1 * op2;
            break;
        case '/':
            result = op1 / op2;
            break;
        default:
            result = 0;
    }

    return result;

}

int main(int argc, char const *argv[]) {

    // server post command line argument
    int server_port = atoi(argv[1]);

    // memory buffer
    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', sizeof(buffer));  // flush buffer

    // shutdown signal
    char shutdown_signal[] = "shutdown";

    // file pointer
    FILE *fptr;

    // create server socket of type TCP
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // define server address - accept any ip address on server port
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // bind server socket to server port
    int binding_status = bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (binding_status == -1) {
        perror("ERROR");
        exit(-1);
    }

    // listen for connections on server port
    listen(server_socket, 1);
    printf("INFO: Server started on port %d.\n", server_port);

    while (1) {
        // accept a connection from client - establish a tunnel
        int connection_socket = accept(server_socket, NULL, NULL);
        printf("INFO: New connection established.\n");

        memset(buffer, '\0', sizeof(buffer));
        ssize_t received_bytes = recv(connection_socket, buffer, sizeof(buffer), 0);
        printf("INFO: Request received (%zd bytes): %s\n", received_bytes, buffer);

        if (memcmp(buffer, shutdown_signal, strlen(shutdown_signal)) == 0) {
            close(connection_socket);
            printf("INFO: Connection closed.\n");
            break;
        }

        int campus_id = (int) strtol(buffer, NULL, 11);
        int y = (campus_id % (((campus_id % 599) + (campus_id % 599)) / 3)) + 98;
        printf("Value of y: %d\n", y);

        fptr = fopen("math.txt", "r");
        if (fptr == NULL) {
            perror("ERROR");
            close(connection_socket);
            printf("INFO: Connection closed.\n");
            continue;
        }

        char c;
        int delim_count = 0;
        while (delim_count != y && !feof(fptr)) {
            fread(&c, sizeof(char), 1, fptr);
            if (c == ';') delim_count++;
        }

        char math_equation[32];
        memset(math_equation, '\0', sizeof(math_equation));
        int math_equation_length = 0;
        while (!feof(fptr)) {
            fread(&c, sizeof(char), 1, fptr);
            if (c == ';') break;
            math_equation[math_equation_length++] = c;
        }

        printf("Delim cout: %d\n", delim_count);

        printf("Math Equation: %s\n", math_equation);

        int result = solve_math_equation(math_equation);
        printf("Result %d\n", result);

        memset(buffer, '\0', sizeof(buffer));
        sprintf(buffer, "%d", result);
        send(connection_socket, buffer, strlen(buffer), 0);

        memset(buffer, '\0', sizeof(buffer));
        received_bytes = recv(connection_socket, buffer, sizeof(buffer), 0);
        printf("INFO: Request received (%zd bytes): %s\n", received_bytes, buffer);

        while (is_lower(buffer) == 0) {
            memset(buffer, '\0', sizeof(buffer));
            strcpy(buffer, "Error");
            send(connection_socket, buffer, strlen(buffer), 0);

            memset(buffer, '\0', sizeof(buffer));
            received_bytes = recv(connection_socket, buffer, sizeof(buffer), 0);
            printf("INFO: Request received (%zd bytes): %s\n", received_bytes, buffer);
        }

        send(connection_socket, buffer, strlen(buffer), 0);

        fclose(fptr);

        close(connection_socket);
        printf("INFO: Connection closed.\n");

    }

    // close server socket
    close(server_socket);
    printf("INFO: Server shutdown.\n");

    return 0;
}
