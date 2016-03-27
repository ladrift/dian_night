/**
 * @file chatroom.c
 * A chatroom server.
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "bind.h"

#define MAXSIZE 256

/**
 * Accept the new connection
 *
 * @param listener the listening socket descriptor
 * @return the new file descriptor
 */
int accept_new_connection(int listener) {
    char remoteIP[INET6_ADDRSTRLEN];
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen = sizeof remoteaddr;

    int new_fd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

    if (new_fd < 0) {
        perror("accept");
    } else {
        printf(
            "chatroom: new connection from %s on "
            "socket %d\n",
            inet_ntop(remoteaddr.ss_family,
                      get_in_addr((struct sockaddr *)&remoteaddr), remoteIP,
                      INET6_ADDRSTRLEN),
            new_fd);
    }
    return new_fd;
}

/**
 * @fn void broadcast_message(int listener, int sender, char *message,
 *                              fd_set members, int fd_max)
 * @brief send message from a sender to all members
 *
 * @param listener the socket descriptor for listen socket
 * @param sender the socket descriptor for sender socket
 * @param members a file descriptor set for alls in chatroom
 * @param fd_max the max file descriptor for all members
 */
void broadcast_message(int listener, int sender, char *message, fd_set members,
                       int fd_max) {
    for (int i = 0; i <= fd_max; ++i) {
        // send to all members
        if (FD_ISSET(i, &members)) {
            // except listener and sender
            if (i != listener && i != sender) {
                asprintf(&message, "member %d: %s", sender, message);
                if (send(i, message, strlen(message), 0) == -1) {
                    perror("send");
                }
                free(message);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: chatroom port\n");
        exit(1);
    }

    int listener = bind_at(argv[1]);

    // `master` matain all the socket descriptor we need to observe
    fd_set master;
    FD_ZERO(&master);
    // add listener to master set
    FD_SET(listener, &master);
    int fd_max = listener;

    // main loop
    while (1) {
        fd_set read_fds = master;
        if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for (int i = 0; i <= fd_max; ++i) {
            if (FD_ISSET(i, &read_fds)) {
                // socket `i` is ready to be read
                if (i == listener) {
                    // handle new connection
                    int new_fd = accept_new_connection(listener);
                    FD_SET(new_fd, &master);
                    if (new_fd > fd_max) {
                        fd_max = new_fd;
                    }
                } else {
                    // handle message received from client
                    char buf[MAXSIZE];
                    int nbytes;
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // error occurred or client close connection
                        if (nbytes == 0) {
                            // connection closed
                            printf("chatroom: socket %d hung up.\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i);
                        FD_CLR(i, &master);
                    } else {
                        // we got some data from client
                        buf[nbytes] = '\0';
                        printf("data from client %d: %s\n", i, buf);

                        broadcast_message(listener, i, buf, master, fd_max);
                    }
                }
            }
        }
    }
    return 0;
}
