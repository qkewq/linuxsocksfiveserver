#include "socksfive.h"
#include <stdint.h>
#include <sys/socket.h>

struct data{
    uint8_t atyp;
    uint8_t addr[255];
    uint8_t port[2];
};

int methodselect(int fd, uint8_t meth){
    uint8_t reply[2] = {0x05, meth};
    if(send(fd, reply, 2, 0) != 2){
        return -1;
    }
    else if(meth == 0xFF){
        return -1;
    }
}

uint8_t con(int fd, uint8_t atyp){
    uint8_t addr[4];
    uint8_t port[2];
    if(atyp == 0x03 || atyp == 0x04){
        return 0x08
    }
    if(recv(fd, addr, 4, MSG_PEEK) != 4){
        0xFF
    }
    if(recv(fd, port, 2, 0) != 2){
        return 0xFF
    }
    return 0x00
}

uint8_t bnd(int fd, uint8_t atyp){
    return 0x07
}

uint8_t udpassoc(int fd, uint8_t atyp){
    return 0x07
}

int socks_handshake(int fd){
    uint8_t buffer[4] = {0};
    uint8_t methods[255] = {0};

    if(recv(fd, buffer, 2, MSG_PEEK) != 2){
        return -1;
    }

    uint8_t version = buffer[0];
    uint8_t num_methods = buffer[1];

    if(version != 0x05){
        return -1;
    }

    if(num_methods < 0x00 || num_methods > 0xFE){
        return -1;
    }

    if(recv(fd, methods, num_methods, 0) != num_methods){
        return -1;
    }

    for(int i = 0; i < num_methods; i++){
        if(methods[i] == 0x00){
            methodselect(fd, 0x00);
            break;
        }
        else{
            methodselect(fd, 0xFF);
            break;
        }
    }

    if(recv(fd, buffer, 4, MSG_PEEK) != 4){
        return -1;
    }

    version = buffer[0];
    uint8_t cmd = buffer [1];
    uint8_t atyp = buffer[3];

    if(version != 0x05){
        return -1;
    }

    if(cmd < 0x01 || cmd > 0x03){
        return -1;
    }

    if(atyp != 0x01 || atyp != 0x03 || atyp != 0x04){
        return -1;
    }

    uint8_t rep = 0xFF

    switch(cmd){
        case 0x01:
            rep = con(fd, atyp);
            break;
        case 0x02:
            rep = bnd(fd, atyp);
            break;
        case 0x03:
            rep = udpassoc(fd, atyp);
            break;
    }





    return 0;
}
