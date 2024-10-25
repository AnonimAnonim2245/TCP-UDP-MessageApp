/*
 * Communication Protocols
 * Client application
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <bits/stdc++.h>

#include <iostream>

#include "common.h"
#include "helpers.h"

using namespace std;
vector<string> topics;  // Stores topics the client is subscribed to



void run_client(int sockfd, char* id) {
    //setvbuf(stdout, NULL, _IOLBF, 0);
    char buf[MSG_MAXSIZE + 1];
    memset(buf, 0, MSG_MAXSIZE + 1);

    struct chat_packet sent_packet;
    struct chat_packet recv_packet;
    struct udp_packet udp_pkt;
    struct client_command spack2;
    struct client_command rpack2;

     /* Set up multiplexing between keyboard input and receiving messages,
       allowing asynchronous handling. */


    struct pollfd poll_fds[2];
    poll_fds[1].fd = sockfd; // Socket for server communication
    poll_fds[1].events = POLLIN; // Monitor input events on socket
    poll_fds[0].fd =  STDIN_FILENO; // Standard input for user commands
    poll_fds[0].events = POLLIN;

    while (1) {
        int rc = poll(poll_fds, 2, -1); // Wait for an event on stdin or socket
        DIE(rc < 0, "poll");
        if (poll_fds[1].revents & POLLIN) {
            
            rc = recv_all(sockfd, &udp_pkt, sizeof(udp_pkt));
            if(rc<=0){
                cout<<"fail.\n";
                return;
            }

            
            
            // Process different data types based on udp_pkt data type
            switch(udp_pkt.data_type){
                case 0:{
                    // Process integer data type
                    uint8_t sign = udp_pkt.value[0];
                    uint32_t num = htonl(*(uint32_t*) (udp_pkt.value+1));
                    if(sign == 1 && num!=0) {
                        printf("%s - INT - -%d\n",udp_pkt.topic, num);
                    } else{
                        printf("%s - INT - %d\n",udp_pkt.topic, num);

                    }

                    break;
                };
                case 1:{
                    // Process short real data type
                    uint16_t num = htons(*(uint16_t*) (udp_pkt.value));
                    float number = (num)/100.00;
                    printf("%s - SHORT_REAL - %.2f\n",udp_pkt.topic, number);

                    break;
                };
                case 2:{
                    // Process floating-point data type
                    uint8_t sign = udp_pkt.value[0];
                    uint32_t num = htonl(*(uint32_t*) (udp_pkt.value+1));
                    uint8_t power = udp_pkt.value[5];

                    double number = num/(pow(10, power));
                    string numberString = to_string(number);
                    size_t pos2 = numberString.find_first_of(".");
                    size_t pos = numberString.find_last_not_of("0");
                    
                    
                    
                    if(sign == 1) {
                        printf("%s - FLOAT - -%.4f\n",udp_pkt.topic, number);
                    } else{
                        printf("%s - FLOAT - %.4f\n",udp_pkt.topic, number);
                    }

        
                    break;

                };
                case 3:{

                    / Process string data type
                    char *val = (char*)malloc((CONTENT_MAXSIZE+1)*sizeof(char));
                    strcpy(val, udp_pkt.value);
                    // Ensure null termination
                    if(strlen(udp_pkt.value) < (CONTENT_MAXSIZE+1)){
                        val[strlen(udp_pkt.value)] ='\0';
                        val = (char*)realloc(val, (strlen(udp_pkt.value)+1) * sizeof(char));
                    } else{
                        val[CONTENT_MAXSIZE+1] ='\0';
                    }
                    printf("%s - STRING - %s\n",udp_pkt.topic, udp_pkt.value);

                    break;
                

                };
            }
        }

        if (poll_fds[0].revents & POLLIN) { // User input from stdin
           
                // don't care about writefds and exceptfds:
          
            std::string str, first, second;
            cin>>first;
            if(!first.compare("exit")){
                break; // User input from stdin
            }
                
            cin>>second;
            strcpy(spack2.command, first.c_str()); // Copy command
            strcpy(spack2.content, second.c_str());  // Copy topic/content
            // Subscribe command     
            if(!first.compare("subscribe")){
                std::cout<<"Subscribed to topic.\n";
                topics.push_back(second);
            } else if (!first.compare("unsubscribe")){ // Unsubscribe command
                std::cout<<"Unsubscribed to topic.\n";
                auto it = find(topics.begin(), topics.end(), second);
                if(it != topics.end()){
                    topics.erase(it);
                }
                        
            } else if(!strcmp(spack2.command, "exit")){
                return;
            } 
            
            // Use send_all function to send the pachet to the server.

            send_all(sockfd, &spack2, sizeof(spack2));
   
        } else{
            continue;
        }

                    
    }
        
}



int main(int argc, char *argv[]) {
    int sockfd = -1;
    // Disables buffering for standard output
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    if (argc != 4) {
        printf("\n Usage: %s <ip> <port>.\n", argv[0]);
        return 1;
    }


    // Parse port number
    uint16_t port;
    int rc = sscanf(argv[3], "%hu", &port);
    DIE(rc != 1, "Given port is invalid");
    
    // Create TCP socket for connecting to the server
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");

    // Disable Nagle's algorithm for low-latency
    int flag = 1;
    int res = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
    if (res < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }


    // Configure server address information
    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);

    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    rc = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);
    DIE(rc <= 0, "inet_pton");

    // Connect to server
    rc = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    DIE(rc < 0, "connect");

    // Send client ID to server
    rc = send(sockfd, argv[1], sizeof(argv[1])+1, 0); 
    DIE(rc < 0, "send");
    
    // Check if the server allows the client to continue
    size_t can_continue;
    rc = recvfrom(sockfd, &can_continue, sizeof(size_t), 0, NULL, NULL);
    DIE(rc < 0, "send");

    // Start the client if the server permits
    if(can_continue) {
        run_client(sockfd, argv[1]);
    }
    

    // Close socket and exit
    close(sockfd);

    return 0;
}
