#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
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

        if(socks_methodselect(sockfd_in, conf) == -1){
            close(sockfd_in);
            continue;//method select error
        }

        if(socks_reqest(sockfd_in, conf) == -1){
            close(sockfd_in);
            continue;//request or reply error
        }

        if(socks_reply(sockfd_in, conf) == -1){
            close(sockfd_in);
            continue;//request or reply error
        }
    }

    return 0;
}
