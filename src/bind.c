/**
 * @file bind.c
 * Functions for bind a server to specified port.
 * @author: ladrift
 * @date: 16-03-20
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BACKLOG 10

/**
 * Handler function `wait()` an exited child process to reap it
 *
 * @param unused_sig the required parameter for signal handler
 */
void sigchld_handler(int unused_sig)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0) {
    }

    errno = saved_errno;
}

/**
 * @fn void *get_in_addr(struct sockaddr *sa)
 * @brief get `in_addr` field from `sockaddr` structure, both IPv4 or IPv6
 *
 * @param sa a socket address structure
 * @return a pointer to struct in_addr or struct in6_addr
 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

/**
 * @fn int bind_at(const char *hostname, const char *port)
 * @brief bind server specified by hostname and port
 *
 * @param hostname a string contains the hostname to server
 * @param port a string contains the port number
 */
int bind_at(const char *port)
{
    struct addrinfo hints;
    struct addrinfo *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  // use my IP

    int rv;
    if (((rv = getaddrinfo(NULL, port, &hints, &servinfo))) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    int sockfd;  // listen on sockfd
    struct addrinfo *p;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
            -1) {
            perror("server: socket");
            continue;
        }

        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes)) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen)) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    // listen
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // Register a handler to reap child processes
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;  // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // print where we were bound
    char remote_ip[INET6_ADDRSTRLEN];
    printf("server: listening at %s:%s\n",
           inet_ntop(p->ai_family, get_in_addr(p->ai_addr), remote_ip,
                     INET6_ADDRSTRLEN),
           port);

    freeaddrinfo(servinfo);

    return sockfd;
}
