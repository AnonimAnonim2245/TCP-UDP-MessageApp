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

int main(){
    char buf[51];
    char buf2[51];
    strcpy(buf, "*/100/+");
    strcpy(buf2, "upb/precis/100/temperature");
    int el = match_elements(buf, buf2);
    cout<<el<<"  ";
    return el;
}
