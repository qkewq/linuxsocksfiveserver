#ifndef SOCKSLIB_H
#include SOCKSLIB_H
struct configs;

int conf_parse(struct configs *conf);
int socks_methodselect(int fd);
int socks_reqestreply(int fd);


#endif
