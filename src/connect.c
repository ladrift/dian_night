/**
 * @file connect.c
 * @brief function for a client to connect a server
 */

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <arpa/inet.h>

/*
 * @fn void *get_in_addr(struct sockaddr *sa)
 * @brief get `in_addr` field from `sockaddr` structure, both IPv4 or IPv6
 *
 * @param sa a socket address structure
 * @return a pointer to struct in_addr or struct in6_addr
 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        // IPv4
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    else {
        // IPv6
        return &(((struct sockaddr_in6 *)sa)->sin6_addr);
    }
}

/**
 * @fn struct addrinfo *get_server_info(char *hostname, char *port)
 * @brief get the server address specified by hostname and port
 *
 * @param hostname a string contains hostname of server
 * @param port the port number where server listening
 * @return status of
 */
struct addrinfo *get_server_info(const char *hostname, const char *port)
{
    // Build a hint to get server's address info
    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *servinfo;
    int rv;
    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return NULL;
    }
    return servinfo;
}

/**
 * @fn int connect_server(const char *hostname, const char *port)
 * @brief connect to the server specified by hostname and port
 *
 * @param hostname a string contains hostname of server
 * @param port the port number where server listening
 * @return the connected sock descriptor
 */
int connect_server(const char *hostname, const char *port)
{
    struct addrinfo *servinfo;
    servinfo = get_server_info(hostname, port);

    // loop through all the results and connect one
    struct addrinfo *p;
    int sockfd;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        // Create a socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
            -1) {
            perror("client: socket");
            // try to use next address
            continue;
        }

        // Connect server
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            // close the un-used socket
            close(sockfd);
            perror("client: connect");
            continue;
        }

        // connect succeed
        break;
    }

    if (p == NULL) {
        // all addresses cannot be connected
        fprintf(stderr, "client: failed to connect\n");
        exit(2);
    }

    // Print out peer's address info
    char s[INET6_ADDRSTRLEN];
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s,
              sizeof s);
    printf("client: connected to %s\n", s);

    // Free the address list
    freeaddrinfo(servinfo);

    return sockfd;
}
