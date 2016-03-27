/**
 * @file bind.h
 *
 * Author: ladrift
 * Date: 16-03-20
 */

#ifndef DIAN_NIGHT_BIND_H
#define DIAN_NIGHT_BIND_H
#include <sys/socket.h>

int bind_at(const char *port);
void *get_in_addr(struct sockaddr *sa);

#endif /* ifndef DIAN_NIGHT_BIND_H */
