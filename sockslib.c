#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "sockslib.h"

int conf_parse(struct configs *conf){//goes in header
    char line[128];
    char key[32];
    char val[64];
    FILE *config_file = fopen("/etc/socksprox.conf", "r");
    if(config_file == NULL){
        return -1;
    }

    while(1 == 1){
        int ikey = 0;
        if(feof(config_file) != 0){
            fclose(config_file);
            return 0;
        }

        if(fgets(line, sizeof(line), config_file) == NULL && feof(config_file) == 0){
            fclose(config_file);
            return -1;
        }
        
        if(line[0] == '\n' || line[0] == '#'){
            continue;
        }

        int delind;
        int linend;
        for(int i = 0; i < sizeof(line); i++){
            if(line[i] == '='){
                delind = i;
            }
            else if(line[i] == '\n'){
                linend = i;
            }
        }

        for(int i = 0; i < delind; i++){
            key[i] = line[i];
        }

        for(int i = 0; i < linend - delind; i++){
            val[i] = line[i + delind];
        }

        if(strncmp(val, "yes", 3) == 0){
            ikey = 1;
        }

        if(strncmp(val, "no", 2) == 0){
            ikey = 2;
        }

        if(strncmp(key, "NOAUTH", sizeof(key) / sizeof(char)) == 0){
            if(ikey == 1){
                conf->smethods.noauth = 0x00;
            }
            else{
                conf->smethods.noauth = 0xFF;
            }
            continue;
        }

        else if(strncmp(key, "USERPASS", sizeof(key) / sizeof(char)) == 0){
            if(ikey == 1){
                conf->smethods.userpass = 0x02;
            }
            else{
                conf->smethods.userpass = 0xFF;
            }
            continue;
        }

        else if(strncmp(key, "GSSAPI", sizeof(key) / sizeof(char)) == 0){
            if(ikey == 1){
                conf->smethods.gssapi = 0x01;
            }
            else{
                conf->smethods.gssapi = 0xFF;
            }
            continue;
        }

        if(strncmp(key, "ipv4", sizeof(key) / sizeof(char)) == 0){
            conf->saddrs.ipver = AF_INET;
            if(inet_pton(AF_INET, key, &conf->saddrs.v4addr) == 1){
                continue;
            }
            else{
                fclose(config_file);
                return -1;
            }
            continue;
        }

        else if(strncmp(key, "ipv6", sizeof(key) / sizeof(char)) == 0){
            conf->saddrs.ipver = AF_INET6;
            if(inet_pton(AF_INET6, key, conf->saddrs.v6addr) == 1){
                continue;
            }
            else{
                fclose(config_file);
                return -1;
            }
            continue;
        }

        if(strncmp(key, "port", sizeof(key) / sizeof(char)) == 0){
            conf->saddrs.portnum = htons(atoi(val));
            continue;
        }
    }

    fclose(config_file);
    return -1;
}

int socks_methodselect(int fd, struct configs *conf){//goes in header
    uint8_t buffer[2];
    uint8_t methods[255];

    if(recv(fd, buffer, 2, MSG_PEEK) != 2){
        return -1;
    }

    if(buffer[0] != 0x05 || buffer[1] != 0x00){
        return -1;
    }

    uint8_t num_methods = buffer[1];
    if(recv(fd, methods, num_methods, 0) != num_methods){
        return -1;
    }

    uint8_t conf_method;
    if(conf->smethods.noauth != 0xFF){
        conf_method == conf->smethods.noauth;
    }

    else if(conf->smethods.userpass != 0xFF){
        conf_method == conf->smethods.userpass;
    }
    
    else if(conf->smethods.gssapi != 0xFF){
        conf_method == conf->smethods.gssapi;
    }

    else{
        conf_method = 0xFF;
    }

    uint8_t reply[2] = {0x05, conf_method};
    if(send(fd, reply, 2, 0) != 2){
        return -1;
    }

    switch(conf_method){
        case 0x00:
            return 0;
        case 0x01:
            return -1;//not supported
        case 0x02:
            return -1;//not supported
        case 0xFF:
            return -1;
        default:
            return -1;
    }
    return -1;
}

int pre_accept_reply(int fd, uint8_t rep, struct configs *conf){
    if(conf->ssreq.atyp == 0x01){
        uint8_t reply[10] = {0x05, rep, 0x00, 0x01, conf->ssreq.v4addr, conf->ssreq.portnum};
        send(fd, reply, 10, 0);
    }
    else if(conf->ssreq.atyp == 0x03){
        uint8_t reply[6 + conf->ssreq.domainlen];
        reply[0] = 0x05;
        reply[1] = rep;
        reply[2] = 0x00;
        reply[3] = 0x03;
        reply[4] = conf->ssreq.domainlen;
        for(int i = 0; i < 6 + conf->ssreq.domainlen; i++){
            reply[5 + i] = conf->ssreq.domain[i];
        }
        reply[5 + conf->ssreq.domainlen] = conf->ssreq.portnum;
        send(fd, reply, 6 + conf->ssreq.domainlen, 0);
    }
    else if(conf->ssreq.atyp == 0x04){
        uint8_t reply[21] = {0x05, rep, 0x00, 0x04};
        for(int i = 0; i < 16; i++){
            reply[4 + i] = conf->ssreq.v6addr[i];
        }
        reply[20] = conf->ssreq.portnum;
        send(fd, reply, 21, 0);
    }
    return 0;
}

int socks_request(int fd, struct configs *conf){//goes in header
    uint8_t buffer[4];
    if(recv(fd, buffer, 4, MSG_PEEK) != 4){
        return -1;
    }

    if(buffer[0] != 0x05 || buffer[2] != 0x00){
        return -1;
    }

    conf->ssreq.cmd = buffer[1];
    conf->ssreq.atyp = buffer[3];

    switch(conf->ssreq.atyp){
        case 0x01:
            if(recv(fd, &conf->ssreq.v4addr, 4, MSG_PEEK) != 4){
                return -1;
            }
            if(recv(fd, &conf->ssreq.portnum, 2, 0) != 2){
                return -1;
            }
            break;
        case 0x03:
            if(recv(fd,& conf->ssreq.domainlen, 1, MSG_PEEK) != 1){
                return -1;
            }
            if(recv(fd, conf->ssreq.domain, conf->ssreq.domainlen, 0) != conf->ssreq.domainlen){
                return -1;
            }
            pre_accept_reply(fd, 0x08, conf);
            return -1;
        case 0x04:
            if(recv(fd, conf->ssreq.v6addr, 16, MSG_PEEK) != 16){
                return -1;
            }
            if(recv(fd, &conf->ssreq.portnum, 2, 0) != 2){
                return -1;
            }
            break;
    }

    switch(conf->ssreq.cmd){
        case 0x01:
            return 0;
        case 0x02:
            pre_accept_reply(fd, 0x07, conf);
            return -1;
        case 0x03:
            pre_accept_reply(fd, 0x07, conf);
            return -1;
    }

    return -1;
}

int socks_reply(int fd, struct configs *conf, uint8_t rep){//goes in header
    if(conf->saddrs.ipver == AF_INET){
        uint8_t reply[10] = {0x05, rep, 0x00, 0x01, conf->soutname.v4addr, conf->soutname.portnum};
        if(send(fd, reply, 10, 0) == -1){
            return -1;
        }
    }
    /*
    else if(conf->ssreq.atyp == 0x03){
        uint8_t reply[6 + conf->ssreq.domainlen] = {0x05, rep, 0x00, 0x03, conf->ssreq.domainlen, conf->ssreq.domaian, conf->ssreq.portnum};
        send(fd, reply, 6 + conf->ssreq.domainlen, 0);
    }
        */
    else if(conf->saddrs.ipver == AF_INET6){
        uint8_t reply[21] = {0x05, rep, 0x00, 0x04, conf->soutname.v6addr, conf->soutname.portnum};
        if(send(fd, reply, 21, 0) == -1){
            return -1;
        }
    }
    else{
        return -1;
    }

    return 0;
}
