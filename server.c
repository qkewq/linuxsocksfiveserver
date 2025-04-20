#include <socksfive.h>
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>

int main(){

//parse config file
//listen on port
//establish tcp connection
//call socks_handshake()
//establish outgiong connection
//add epoll
//start passing data

    struct sockaddr_in sockin;
    memset(&sockin, 0, sizeof(sockin));
    sockin.sin_family = AFINET;
    sockin.sin_port = //PORT
    sockin.sin_addr.s_addr = //IP

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1){
        return 1;
    }

    if(bind(fd, &sockin, sizeof(sockin)) == -1){
        close(fd);
        return 1;
    }

    if(listen(fd, SOMAXCONN) == -1){
        close(fd);
        return 1;
    }

    struct sfivedata sfdata;
    memset(&sfdata, 0, sizeof(sfdata));

    while(1 == 1){
        struct sockaddr_in connsock;
        memset(&connsock, 0, sizeof(connsock));

        int cfd = accept(fd, NULL, NULL);
        if(socks_handshake(cfd, &sfdata) == -1){
            close(cfd);
            continue;
        }

        connsock.sin_family = AFINET;
        memccpy(&connsock.sin_port, sfdata.port, 2);
        memcpy(&connsock.sin_addr, sfdata.addrv4, 4);

        int outfd = socket(AF_INET, SOCK_STREAM, 0);
        if(outfd == -1){
            close(cfd);
            continue;
        }

        if(connect(cfd, &connsock, sizeof(connsock)) == -1){
            close(cfd);
            continue;
        }

        if(socks_successreply(cfd, &sfdata) == -1){
            close(cfd);
            continue;
        }

        //epoll

    }

    return 0;
}
