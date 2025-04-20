#ifndef SOCKSFIVE_H

#define SOCKSFIVE_H

struct sfivedata;

int socks_handshake(int fd, struct sfivedata *sfdata);

int socks_successreply(int fd, struct sfivedata *sfdata);

#endif
