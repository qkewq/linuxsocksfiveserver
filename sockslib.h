#ifndef SOCKSLIB_H
#define SOCKSLIB_H
struct configs;

int conf_parse(struct configs *conf);
int socks_methodselect(int fd, struct configs *conf);
int socks_reqest(int fd, struct configs *conf);
int socks_reply(int fd, struct configs *conf);


#endif
