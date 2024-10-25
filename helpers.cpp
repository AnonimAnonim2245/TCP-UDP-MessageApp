#include <arpa/inet.h>
#include <errno.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <algorithm>
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
int match_elements(const char* sub, char* topic){


    std::string subStr(sub);
    string topicStr(topic);
    char buf[51];

    int num1 = count(subStr.begin(),subStr.end(), '/')+1;
    int num2 = count(topicStr.begin(), topicStr.end(), '/')+1;
    if(num1 == 0 | num2 == 0){
        return 0;
    }

    
    strcpy(buf, sub);
    buf[strlen(sub)] = '\0';
    string str1, str2;
    int ok = 1, pos1, pos2;
    pos1 = subStr.find("/");
    pos2 = topicStr.find("/");
    
    str1 = subStr.substr(0, pos1);
    str2 = topicStr.substr(0, pos2);
    int totalPos1=0, totalPos2=0;

    while(ok==1){
        if((num1 ==0  ||  num2==0) && num1!=num2){
            ok = 0;
            continue;
        }
        ok=0;
        if(!str1.compare(str2)){
            //cout<<str1<<"  "<<str2<<endl;

            ok = 1;
            subStr=subStr.substr(pos1+1);
            topicStr=topicStr.substr(pos2+1);

            pos1 = subStr.find("/");
            pos2 = topicStr.find("/");
    
            str1 = subStr.substr(0, pos1);
            str2 = topicStr.substr(0, pos2);

            num1-=1;
            num2-=1;

        } else if(!str1.compare("+")){
            ok = 1;
            subStr=subStr.substr(pos1+1);
            topicStr=topicStr.substr(pos2+1);
            pos1 = subStr.find("/");
            pos2 = topicStr.find("/");
    
            str1 = subStr.substr(0, pos1);
            str2 = topicStr.substr(0, pos2);
            num1-=1;
            num2-=1;

        } else if(!str1.compare("*") && num1 <= num2){
            ok=1;
            
            topicStr=topicStr.substr(pos2+1);
            pos2 = topicStr.find("/");

            str2 = topicStr.substr(0, pos2);
            num2-=1;
            
            string tempStr2 = subStr.substr(pos1+1);
            int tempPos = tempStr2.find("/");
            string tempStr1 = tempStr2.substr(0, tempPos);
        

            if(num1==num2+1 || !tempStr1.compare(str2)){
                str1 = tempStr1;  
                subStr = tempStr2;
                pos1 = tempPos;              
                num1-=1;
            }

        }
        

        if(num1 == 0 && num2==0){
            ok = 2;
        }

    }
    return ok;
}

char* buildStatement(char* name, struct udp_packet udp_pkt){
    //htonl
    strcpy(name, udp_pkt.topic);
    switch(udp_pkt.data_type){
        case 0:{

            strcat(name," - INT - ");
            uint8_t sign = udp_pkt.value[0];
            uint32_t num = htonl(*(uint32_t*) (udp_pkt.value+1));
            if(sign == 1) {
                strcat(name, "-");
            }
            strcat(name, to_string(num).c_str());
            strcat(name, udp_pkt.value);
            break;
        };
        case 1:{
            strcat(name," - SHORT_REAL - ");
            uint16_t num = htons(*(uint16_t*) (udp_pkt.value));
            float number = (num)/100.00;
            std::stringstream ss;
            ss << std::fixed<< setprecision(2) << number;
            strcat(name, ss.str().c_str());
            break;
        };
        case 2:{
            strcat(name, " - FLOAT - ");
            uint8_t sign = udp_pkt.value[0];
            uint32_t num = htonl(*(uint32_t*) (udp_pkt.value+1));
            uint8_t power = udp_pkt.value[5];

            if(sign == 1) {
                strcat(name, "-");
            }

            double number = num/(pow(10, power));
            string numberString = to_string(number);
            size_t pos2 = numberString.find_first_of(".");
            size_t pos = numberString.find_last_not_of("0");

            std::stringstream ss;
            ss << std::fixed<< setprecision(pos-pos2) << number;
            strcat(name, ss.str().c_str());
            break;

        };
        case 3:{
            strcat(name, " - STRING - ");
            char *val = (char*)malloc((CONTENT_MAXSIZE+1)*sizeof(char));
            strcpy(val, udp_pkt.value);
            if(strlen(udp_pkt.value) < (CONTENT_MAXSIZE+1)){
                val[strlen(udp_pkt.value)] ='\0';
                val = (char*)realloc(val, (strlen(udp_pkt.value)+1) * sizeof(char));
            } else{
                val[CONTENT_MAXSIZE+1] ='\0';
            }
            strcat(name, val);
            break;
        

        };
    }
    return name;
}