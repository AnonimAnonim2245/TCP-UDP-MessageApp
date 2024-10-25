#include <arpa/inet.h>
#include <errno.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#include "helpers.h"
using namespace std;

string buildStatement(string name, struct udp_packet udp_pkt){
    //htonl
    name+=+udp_pkt.topic+string(" - ");
    switch(udp_pkt.data_type){
        case 0:{

            name+="INT - ";
            uint8_t sign = udp_pkt.value[0];
            uint32_t num = htonl(*(uint32_t*) (udp_pkt.value+1));
            if(sign == 1) {
                name+=string("-");
            }
            name+=to_string(num);
            name+=udp_pkt.value;
            break;
        };
        case 1:{
            name+="SHORT_REAL - ";
            uint16_t num = htons(*(uint16_t*) (udp_pkt.value));
            float number = (num)/100.00;
            std::stringstream ss;
            ss << std::fixed<< setprecision(2) << number;
            name+=ss.str();
            break;
        };
        case 2:{
            name+="FLOAT - ";
            uint8_t sign = udp_pkt.value[0];
            uint32_t num = htonl(*(uint32_t*) (udp_pkt.value+1));
            uint8_t power = udp_pkt.value[5];

            if(sign == 1) {
                name+=string("-");
            }

            double number = num/(pow(10, power));
            string numberString = to_string(number);
            size_t pos2 = numberString.find_first_of(".");
            size_t pos = numberString.find_last_not_of("0");

            std::stringstream ss;
            ss << std::fixed<< setprecision(pos-pos2) << number;
            name+=ss.str();
            break;

        };
        case 3:{
            name+="STRING - ";
            char *val = (char*)malloc((CONTENT_MAXSIZE+1)*sizeof(char));
            strcpy(val, udp_pkt.value);
            if(strlen(udp_pkt.value) < (CONTENT_MAXSIZE+1)){
                val[strlen(udp_pkt.value)] ='\0';
                val = (char*)realloc(val, (strlen(udp_pkt.value)+1) * sizeof(char));
            } else{
                val[CONTENT_MAXSIZE+1] ='\0';
            }
            name+=string(val);

        };
    }
    return name;
}