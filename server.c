#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "sockslib.h"

int main(void){
    struct configs conf;
    memset(&conf, 0, sozeof(conf));

    if(conf_parse(&conf) == -1){
        return 1;//config error
    }

    int sockfd;
    if(conf.saddrs.ipver == AF_INET){
        struct sockaddr_in sock;
        memset(&sock, 0, sozeof(sock));
        sock.sin_family = AF_INET;
        sock.sin_port = conf.sadders.portnum;
        sock.sin_addr.s_addr = conf.sadders.v4addr;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd == -1){
            return 1;//sock error
        }
    }

    else if(conf.saddrs.ipver == AF_INET6){
        struct sockaddr_in6 sock;
        memset(&sock, 0, sozeof(sock));
        sock.sin6_family = AF_INET6;
        sock.sin6_port = conf.sadders.portnum;
        sock.sin6_addr.s6_addr = conf.saddrs.v6addr;
        sockfd = socket(AF_INET6, SOCK_STREAM, 0);
        if(sockfd == -1){
            return 1;//sock error
        }
    }

    if(bind(sockfd, &sock, sizeof(sock)) == -1){
        close(sockfd);
        return 1;//bind error
    }

    if(listen(sockfd, SOMAXCONN) == -1){
        close(sockfd);
        return 1;//listen error
    }

    while(1 == 1){
        int sockfd_in = accept(sockfd, NULL, NULL);
        if(sockfd_in == -1){
            close(sockfd);
            return 1;//accept error
        }

        if(socks_methodselect(sockfd_in, &conf) == -1){
            close(sockfd_in);
            continue;//method select error
        }

        if(socks_reqest(sockfd_in, &conf) == -1){
            close(sockfd_in);
            continue;//request error
        }

        int sockfd_out;
        if(conf.ssreq.atyp == 0x01){
            struct sockaddr_in sockout;
            memset(&sockout, 0, sozeof(sockout));
            sock.sin_family = AF_INET;
            sock.sin_port = conf.ssreq.portnum;
            sock.sin_addr.s_addr = conf.ssreq.v4addr;
            sockfd_out = socket(AF_INET, SOCK_STREAM, 0);
            if(sockfd_out == -1){
                continue;//sock error
            }
        }
    
        else if(conf.ssreq.atyp == 0x04){
            struct sockaddr_in6 sockout;
            memset(&sockout, 0, sozeof(sockout));
            sock.sin6_family = AF_INET6;
            sock.sin6_port = conf.ssreq.portnum;
            sock.sin6_addr.s6_addr = conf.ssreq.v6addr;
            sockfd_out = socket(AF_INET6, SOCK_STREAM, 0);
            if(sockfd_out == -1){
                continue;//sock error
            }
        }

        uint8_t rep;
        int conn = connect(sockfd_out, &sockout, sizeof(sockout));
        if(getsockname(sockfd_out, &sockout, sizeof(sockout)) == -1){
            close(sockfd_out);
            continue;
        }

        if(conf.saddrs.ipver == AF_INET){
            conf.soutname.v4addr = sockout.sin_addr.s_addr;
            conf.soutname.portnum = sockout.sin_port;
        }

        else if(conf.saddrs.ipver == AF_INET6){
            conf.soutname.v6addr = sockout.sin6_addr.s6_addr;
            conf.soutname.portnum = sockout.sin6_port;
        }

        if(conn == -1){
            switch(errno){
                case ENETUNREACH:
                    rep = 0x03;
                    break;
                case ECONNREFUSED:
                    rep = 0x05;
                    break;
                default:
                    rep = 0x01;
                    break;
            }
            socks_reply(sockfd_in, rep, *conf);
            close(sockfd_in);
            continue;
        }

        else{
            if(socks_reply(sockfd_in, 0x00, *conf) == -1){
                close(sockfd_out);
                close(sockfd_in);
            }
        }

        //epoll
    }

    return 0;
}
