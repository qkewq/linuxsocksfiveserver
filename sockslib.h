#ifndef SOCKSLIB_H
#define SOCKSLIB_H

struct methods{
    uint8_t noauth;
    uint8_t userpass;
    uint8_t gssapi;

};

struct addrs{
    int ipver;
    uint32_t v4addr;
    char v6addr[16];
    uint16_t portnum;
};

struct sockreq{
    uint8_t cmd;
    uint8_t atyp;
    uint32_t v4addr;
    char v6addr[16];
    uint16_t portnum;
    uint8_t domainlen;
    char domain[255];
};

struct outname{
    uint32_t v4addr;
    char v6addr[16];
    uint16_t portnum;
};

struct configs{
    struct methods smethods;
    struct addrs saddrs;
    struct sockreq ssreq;
    struct outname soutname;
};

int conf_parse(struct configs *conf);
int socks_methodselect(int fd, struct configs *conf);
int socks_request(int fd, struct configs *conf);
int socks_reply(int fd, struct configs *conf, uint8_t rep);

#endif
