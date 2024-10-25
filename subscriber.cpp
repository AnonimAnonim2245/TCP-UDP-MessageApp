/*
 * Protocoale de comunicatii
 * Laborator 7 - TCP si mulplixare
 * client.c
 */

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
#include<vector>
#include<unordered_map>
#include <iostream>
using namespace std;
#include "common.h"
#include "helpers.h"

vector<string> topics;


void run_client(int sockfd, char* id) {


    char buf[MSG_MAXSIZE + 1];
    memset(buf, 0, MSG_MAXSIZE + 1);

    struct chat_packet sent_packet;
    struct chat_packet recv_packet;

    struct client_command spack2;
    struct client_command rpack2;

    /* 2.2: Multiplexeaza intre citirea de la tastatura si primirea unui
       mesaj, ca sa nu mai fie impusa ordinea.
    */


    struct pollfd poll_fds[2];
    poll_fds[1].fd = sockfd;
    poll_fds[1].events = POLLIN;
    poll_fds[0].fd = fileno(stdin);
    poll_fds[0].events = POLLIN;

    while (1) {
        int rc = poll(poll_fds, 2, -1);
        DIE(rc < 0, "poll");

        

        if (poll_fds[0].revents & POLLIN) {
            if (fgets(buf, sizeof(buf), stdin)) {

                if(buf[strlen(buf)-1]=='\n'){
                    buf[strlen(buf)-1] = '\0';
                }
                char *element;
                spack2.header.len = strlen(buf) + 1;
                element = strtok(buf, " ");
                strcpy(spack2.command, element);
                    
                element = strtok(NULL, " ");
                if(element != NULL){
                    strcpy(spack2.content, element);
                }
                    
                
                        // Use send_all function to send the pachet to the server.
                        
                send_all(sockfd, &spack2, sizeof(spack2));
                
                
                if(!strncmp(spack2.command, "subscribe", 9)){
                    std::cout<<"Subscribed to topic.\n";
                    string content(element);
                    topics.push_back(content);     
                } else if (!strncmp(spack2.command, "unsubscribe", 11)){
                    std::cout<<"Unsubscribed to topic.\n";
                    string content(element);
                    auto it = std::find(topics.begin(), topics.end(), content);
                    if(it != topics.end()){
                        topics.erase(it);
                    }
                    
                } else if(!strncmp(spack2.command, "exit", 4)){
                    return;
                } else {
                    cout<<"Invalid command.\n";
                    continue;
                }
                
                
                    
                
            } else {
                cout<<"???!!!!.\n";
                break;
            }
        }
        if (poll_fds[1].revents & POLLIN) {
            size_t charLen;
            rc = recv(sockfd, &charLen, sizeof(size_t), 0);
            DIE(rc < 0, "recv");
            
            char *elementChar = new char[charLen];
            rc = recv_all(sockfd, elementChar, sizeof(elementChar));
            DIE(rc <0, "recv_all");
            cout<<elementChar<<".\n";
            continue;
        }
    }

    
}

int main(int argc, char *argv[]) {
    int sockfd = -1;
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    if (argc != 4) {
        printf("\n Usage: %s <ip> <port>.\n", argv[0]);
        return 1;
    }
    // Parsam port-ul ca un numar
    uint16_t port;
    int rc = sscanf(argv[3], "%hu", &port);
    DIE(rc != 1, "Given port is invalid");
    /**/
    // Obtinem un socket TCP pentru conectarea la server
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");

    // Completăm in serv_addr adresa serverului, familia de adrese si portul
    // pentru conectare
    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);

    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    rc = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);
    DIE(rc <= 0, "inet_pton");

    // Ne conectăm la server
    rc = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    DIE(rc < 0, "connect");
    rc = send(sockfd, argv[1], sizeof(argv[1])+1, 0); 
    DIE(rc < 0, "send");
    
    run_client(sockfd, argv[1]);

    // Inchidem conexiunea si socketul creat
    close(sockfd);

    return 0;
}
