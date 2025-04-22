#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "sockslib.h"

struct configs{//goes in header
    struct methods smethods;
    struct addrs saddrs;
};

struct methods{
    int noauth;
    int userpass;
    int gssapi;

};

struct addrs{
    int ipver;
    uint32_t v4addr;
    __uint128_t v6addr;
    uint16_t portnum;
}

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
        
        if(line[0] = "\n" || line[0] == "#"){
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
                conf->smethods.noauth = 1;
            }
            else{
                conf->smethods.noauth = 0;
            }
            continue;
        }

        else if(strncmp(key, "USERPASS", sizeof(key) / sizeof(char)) == 0){
            if(ikey == 1){
                conf->smethods.userpass = 1;
            }
            else{
                conf->smethods.userpass = 0;
            }
            continue;
        }

        else if(strncmp(key, "GSSAPI", sizeof(key) / sizeof(char)) == 0){
            if(ikey == 1){
                conf->smethods.gssapi = 1;
            }
            else{
                conf->smethods.gssapi = 0;
            }
            continue;
        }

        if(strncmp(key, "ipv4", sizeof(key) / sizeof(char)) == 0){
            conf->saddrs.ipver = AF_INET
            if(inet_pton(AF_INET, key, conf->saddrs.v4addr) == 1){
                continue;
            }
            else{
                fclose(config_file);
                return -1;
            }
            continue;
        }

        else if(strncmp(key, "ipv6", sizeof(key) / sizeof(char)) == 0){
            conf->saddrs.ipver = AF_INET6
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
