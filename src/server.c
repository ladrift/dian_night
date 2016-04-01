/**
 * @file server.c
 * @brief a server do basic calculation task
 *
 * Author: ladrift
 * Date: 16-03-20
 */

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "bind.h"

#define MAXSIZE 100

char remote_ip[INET6_ADDRSTRLEN];

/**
 * @fn int strisfloat(const char *operand)
 * @brief check whether a str is a valid float number
 *
 * @param operand a string contains the operand
 * @return a integer, 1 means true, 0 means false
 */
int strisfloat(const char *operand)
{
    int len = strlen(operand);
    for (int i = 0; i < len; ++i) {
        if (!isdigit(operand[i]) && operand[i] != '.') {
            return 0;
        }
    }

    return 1;
}

/**
 * @fn double calculate(char *buf, char **err_msg)
 * @brief calculate the result of command in buf
 *
 * @param buf the string of command message
 * @param err_msg a pointer to pointer to where to
 *                put error message while processing command
 * @return the result of calculation
 */
double calculate(char *buf, char **err_msg)
{
    // Retrive the comman
    char *op_code = strtok(buf, " ");
    char *operand1 = NULL;
    char *operand2 = NULL;
    if (op_code) {
        operand1 = strtok(NULL, " ");
        operand2 = strtok(NULL, " ");
    }

    // char *err_msg = NULL;
    double op1 = NAN;
    double op2 = NAN;
    double result = NAN;

    if (operand1 && operand2) {
        if (!strisfloat(operand1)) {
            asprintf(err_msg, "First operand '%s' is not a float number.",
                     operand1);
        }
        else if (!strisfloat(operand2)) {
            asprintf(err_msg, "Second operand '%s' is not a float number.",
                     operand2);
        }
        else {
            op1 = atof(operand1);
            op2 = atof(operand2);

            if (strcmp(op_code, "add") == 0) {
                result = op1 + op2;
            }
            else if (strcmp(op_code, "sub") == 0) {
                result = op1 - op2;
            }
            else if (strcmp(op_code, "mul") == 0) {
                result = op1 * op2;
            }
            else if (strcmp(op_code, "div") == 0) {
                result = op1 / op2;
            }
            else {
                asprintf(err_msg, "Command '%s' not exists", op_code);
            }
        }
    }
    else {
        asprintf(err_msg, "Operands not enough (need two operands)");
    }

    return result;
}

/**
 * @fn void handle_client(int fd)
 * @brief handle the new client
 *
 * @param newfd a new socket descriptor to conmunicate with client
 */
void handle_client(int fd)
{
    while (1) {
        // Receive message from client
        int numbytes;
        char buf[MAXSIZE];
        if ((numbytes = recv(fd, buf, MAXSIZE - 1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        else if (numbytes == 0) {
            printf("connection from %s closed\n", remote_ip);
            exit(0);
        }

        buf[numbytes] = '\0';
        printf("message received from client: %s\n", buf);

        char *err_msg;
        double result = calculate(buf, &err_msg);

        char *response;
        if (isnan(result)) {
            asprintf(&response, "1 %s", err_msg);
        }
        else {
            asprintf(&response, "0 %g", result);
        }

        // Send response to client
        if (send(fd, response, strlen(response), 0) == -1) {
            perror("send");
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: server port\n");
        exit(1);
    }

    int sockfd = bind_at(argv[1]);
    while (1) {
        // main accept() loop
        int new_fd;
        struct sockaddr_storage their_addr;
        socklen_t sin_size = sizeof their_addr;

        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr), remote_ip,
                  sizeof remote_ip);
        printf("server: got connection from %s\n", remote_ip);

        if (!fork()) {      // this is the child process
            close(sockfd);  // child doesn't need the listener

            handle_client(new_fd);

            // child process exit
            close(new_fd);
            exit(0);
        }
        close(new_fd);
    }

    return 0;
}
