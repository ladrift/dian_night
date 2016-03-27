#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connect.h"

#define MAXSIZE 100

/**
 * @fn int get_line(char *message)
 * @brief Get a line of input from stdin
 *
 * @param message a pointer to buffer where to store the line
 * @return the number of characters stored to the buffer
 */
int get_line(char *message) {
    int num_ch = 0;
    int c;
    while ((c = getchar()) != EOF && c != '\n') {
        num_ch++;
        *message++ = c;
    }

    return num_ch;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: chat_client hostname port\n");
        exit(1);
    }

    int sockfd = connect_server(argv[1], argv[2]);

    fd_set observed;
    FD_ZERO(&observed);

    // add stdin to observed set
    FD_SET(STDIN_FILENO, &observed);
    FD_SET(sockfd, &observed);

    int fd_max = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

    while (1) {
        fd_set read_fds = observed;
        if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        // run through the existing connections looking for data to read
        for (int i = 0; i <= fd_max; ++i) {
            if (FD_ISSET(i, &read_fds)) {
                // socket `i` is ready to be read
                char buf[MAXSIZE];
                int nbytes;
                if (i == STDIN_FILENO) {
                    // Read message from stdin
                    nbytes = get_line(buf);

                    // send message to server
                    if (send(sockfd, buf, nbytes, 0) == -1) {
                        perror("send");
                    }
                } else if (i == sockfd) {
                    // Receive message from server
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        if (nbytes == 0) {
                            // connection closed
                            fprintf(stderr, "chat_client: server closed.\n");
                            exit(1);
                        } else {
                            perror("recv");
                        }
                    }

                    // send message to stdout
                    buf[nbytes] = '\0';
                    printf("%s\n", buf);
                }
            }
        }
    }
    return 0;
}
