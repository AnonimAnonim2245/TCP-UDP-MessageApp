#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <bits/stdc++.h>

#include <iostream>
using namespace std;
#include "common.h"
#include "helpers.h"
class Database{
    public:
        vector<string> topics;
    
    void updateTopics(string Topic){
        topics.push_back(Topic);
    }

    vector<string> getTopic(){
        return topics;
    }

};