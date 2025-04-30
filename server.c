#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

#include "sockslib.h"


int main(void){
    struct configs conf;
    memset(&conf, 0, sizeof(conf));

    if(conf_parse(&conf) == -1){
        printf("config error: %s\n", strerror(errno));
        return 1;//config error
    }

    struct sockaddr_in sock;
    memset(&sock, 0, sizeof(sock));

    struct sockaddr_in6 sock_6;
    memset(&sock_6, 0, sizeof(sock_6));

    if(conf.saddrs.ipver == AF_INET){
        sock.sin_family = AF_INET;
        sock.sin_port = conf.saddrs.portnum;
        sock.sin_addr.s_addr = conf.saddrs.v4addr;
    }

    else if(conf.saddrs.ipver == AF_INET6){
        sock_6.sin6_family = AF_INET6;
        sock_6.sin6_port = conf.saddrs.portnum;
        //sock.sin6_addr.s6_addr = conf->saddrs.v6addr;
        memcpy(sock_6.sin6_addr.s6_addr, conf.saddrs.v6addr, 16);
    }

    int sockfd;
    if(conf.saddrs.ipver == AF_INET){
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd == -1){
            printf("in socket create error: %s\n", strerror(errno));
            return 1;//sock error
        }
    }

    else if(conf.saddrs.ipver == AF_INET6){
        sockfd = socket(AF_INET6, SOCK_STREAM, 0);
        if(sockfd == -1){
            printf("in socket create error: %s\n", strerror(errno));
            return 1;//sock error
        }
    }

    if(conf.saddrs.ipver == AF_INET){
        if(bind(sockfd, (struct sockaddr *)&sock, sizeof(sock)) == -1){
            close(sockfd);
            printf("bind error: %s\n", strerror(errno));
            return 1;//bind error
        }
    }

    else if(conf.saddrs.ipver == AF_INET6){
        if(bind(sockfd, (struct sockaddr *)&sock_6, sizeof(sock_6)) == -1){
            close(sockfd);
            printf("bind error: %s\n", strerror(errno));
            return 1;//bind error
        }
    }

    if(listen(sockfd, SOMAXCONN) == -1){
        close(sockfd);
        printf("listen error: %s\n", strerror(errno));
        return 1;//listen error
    }

    while(1 == 1){
        int sockfd_in = accept(sockfd, NULL, NULL);
        if(sockfd_in == -1){
            close(sockfd);
            printf("accept error: %s\n", strerror(errno));
            return 1;//accept error
        }

        if(socks_methodselect(sockfd_in, &conf) == -1){
            close(sockfd_in);
            printf("method select error: %s\n", strerror(errno));
            continue;//method select error
        }

        if(socks_request(sockfd_in, &conf) == -1){
            close(sockfd_in);
            printf("socks request error: %s\n", strerror(errno));
            continue;//request error
        }

        struct sockaddr_in sockout;
        memset(&sockout, 0, sizeof(sockout));

        struct sockaddr_in6 sockout_6;
        memset(&sockout_6, 0, sizeof(sockout_6));

        if(conf.ssreq.atyp == 0x01){
            sockout.sin_family = AF_INET;
            sockout.sin_port = conf.ssreq.portnum;
            sockout.sin_addr.s_addr = conf.ssreq.v4addr;
        }

        else if(conf.ssreq.atyp == 0x04){
            sockout_6.sin6_family = AF_INET6;
            sockout_6.sin6_port = conf.ssreq.portnum;
            //sock.sin6_addr.s6_addr = conf->ssreq.v6addr;
            memcpy(sockout_6.sin6_addr.s6_addr, conf.ssreq.v6addr, 16);
        }

        int sockfd_out;
        if(conf.ssreq.atyp == 0x01){
            sockfd_out = socket(AF_INET, SOCK_STREAM, 0);
            if(sockfd_out == -1){
                printf("out socket create error: %s\n", strerror(errno));
                continue;//sock error
            }
        }
    
        else if(conf.ssreq.atyp == 0x04){
            sockfd_out = socket(AF_INET6, SOCK_STREAM, 0);
            if(sockfd_out == -1){
                printf("out socket create error: %s\n", strerror(errno));
                continue;//sock error
            }
        }

        uint8_t rep;
        int conn = -1;
        if(conf.ssreq.atyp == 0x01){
            socklen_t addrlen = sizeof(sockout);
            conn = connect(sockfd_out, (struct sockaddr *)&sockout, sizeof(sockout));
            if(getsockname(sockfd_out, (struct sockaddr *)&sockout, &addrlen) == -1){
                close(sockfd_out);
                printf("connect error: %s\n", strerror(errno));
                continue;//connect error
            }
        }

        else if(conf.ssreq.atyp == 0x04){
            socklen_t addrlen = sizeof(sockout_6);
            conn = connect(sockfd_out, (struct sockaddr *)&sockout_6, sizeof(sockout_6));
            if(getsockname(sockfd_out, (struct sockaddr *)&sockout_6, &addrlen) == -1){
                close(sockfd_out);
                printf("connect error: %s\n", strerror(errno));
                continue;//connect error
            }
        }

        if(conf.saddrs.ipver == AF_INET){
            memcpy(conf.soutname.v4addr, &sockout.sin_addr.s_addr, 4);
            memcpy(conf.soutname.portnum, &sockout.sin_port, 2);
        }

        else if(conf.saddrs.ipver == AF_INET6){
            memcpy(conf.soutname.v6addr, sockout_6.sin6_addr.s6_addr, 16);
            // conf.soutname.portnum = sockout_6.sin6_port;
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
            socks_reply(sockfd_in, &conf, rep);
            close(sockfd_in);
            continue;
        }

        else{
            if(socks_reply(sockfd_in, &conf, 0x00) == -1){
                close(sockfd_out);
                close(sockfd_in);
                printf("socks reply error: %s\n", strerror(errno));
                continue;//reply error
            }
        }

        int epfd = epoll_create1(0);
        if(epfd == -1){
            close(sockfd_out);
            close(sockfd_in);
            printf("epoll create error: %s\n", strerror(errno));
            continue;//epoll create error
        }

        struct epoll_event epin;
        struct epoll_event epout;
        memset(&epin, 0, sizeof(epin));
        memset(&epout, 0, sizeof(epout));

        epin.events = EPOLLIN | EPOLLET;
        epout.events = EPOLLIN | EPOLLET;
        epin.data.fd = sockfd_in;
        epout.data.fd = sockfd_out;

        if(epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd_in, &epin) == -1 || epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd_out, &epout) == -1){
            close(sockfd_out);
            close(sockfd_in);
            close(epfd);
            printf("epoll ctl error: %s\n", strerror(errno));
            continue;//epoll ctl error
        }

        if(fcntl(sockfd_in, F_SETFD, O_NONBLOCK) == -1 || fcntl(sockfd_in, F_SETFD, O_NONBLOCK) == -1){
            close(sockfd_out);
            close(sockfd_in);
            close(epfd);
            printf("set nonblock error: %s\n", strerror(errno));
            continue;//set nonblock error
        }

        struct epoll_event events[2];
        while(1 == 1){
            uint8_t buffer[4096] = {0};
            int epwait = epoll_wait(epfd, events, 2, -1);
            if(epwait == -1){
                close(sockfd_out);
                close(sockfd_in);
                close(epfd);
                printf("epoll wait error: %s\n", strerror(errno));
                break;//epoll wait error
            }


            for(int i = 0; i < epwait; i++){
                int read_readyfd = events[i].data.fd;
                int send_readyfd;

                if(read_readyfd == sockfd_in){
                    send_readyfd = sockfd_out;
                }

                else if(read_readyfd == sockfd_out){
                    send_readyfd = sockfd_in;
                }

                if(events[i].events & EPOLLIN){
                    int reclen = recv(read_readyfd, buffer, sizeof(buffer), 0);
                    if(reclen <= 0){
                        close(sockfd_out);
                        close(sockfd_in);
                        close(epfd);
                        printf("epoll read error: %s\n", strerror(errno));
                        break;//epoll read error
                    }
                    if(send(send_readyfd, buffer, reclen, 0) == -1){
                        close(sockfd_out);
                        close(sockfd_in);
                        close(epfd);
                        printf("epoll send error: %s\n", strerror(errno));
                        break;//epoll send error
                    }
                }
            }
        }
    }
    return 0;
}
