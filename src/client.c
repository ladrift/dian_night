/*
 * @file client.c
 * The client for Dian night.
 *
 * The client using TCP to send message to a server. The server's IP/hostname
 * and port number need to be specified in command line.
 *
 * @author: ladrift
 * @date: 2016-03-19
 */

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "connect.h"

#define MAXSIZE 100

/**
 * Get a line of input from `stdin`
 *
 * @param message a pointer to buffer where to store the line
 * @return the number of characters stored to the buffer
 */
int get_line(char *message)
{
    int num_ch = 0;
    int c;
    while ((c = getchar()) != EOF && c != '\n') {
        num_ch++;
        *message++ = c;
    }

    return num_ch;
}

int main(int argc, char **argv)
{
    // Check commandline arguments
    if (argc != 3) {
        fprintf(stderr, "usage: client hostname port\n");
        exit(1);
    }

    int sockfd = connect_server(argv[1], argv[2]);

    while (!feof(stdin)) {
        // Read client's message from `stdin`
        char message[MAXSIZE];
        printf("client: Enter your message to server: ");
        int len = get_line(message);

        // Send client's message to server
        int bytes_sent;
        if ((bytes_sent = send(sockfd, message, len, 0)) == -1) {
            perror("send");
            exit(1);
        }

        // Read message sent from server
        int numbytes;
        char buf[MAXSIZE];
        if ((numbytes = recv(sockfd, buf, MAXSIZE - 1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        else if (numbytes == 0) {
            printf("server closed.\n");
            exit(1);
        }

        buf[numbytes] = '\0';

        if (buf[0] == '0') {
            printf("client: result from server: %s\n", buf + 2);
        }
        else {
            printf("client: error from server: %s\n", buf + 2);
        }
    }

    close(sockfd);

    return 0;
}
