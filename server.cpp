/*
 * Protocoale de comunicatii
 * Laborator 7 - TCP
 * Echo Server
 * server.c
 */

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
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <string_view>

#include "common.h"
#include "helpers.h"

#define MAX_CONNECTIONS 32
#define FULLMSG_SIZE 2000
using namespace std;

// Namespace and data structures for client-topic relationships
// find all the topics of a specific client id
unordered_map<string, vector<string>> clientid_to_topic;
// all topics
vector<string> topicsTotal;
// find an id by socket
unordered_map<int, string> socket_to_id;
// reverse
unordered_map<string, int> id_to_socket;


void run_chat_multi_server(int listenfd, int listen_udp) {

    struct pollfd poll_fds[MAX_CONNECTIONS];
    int num_clients = 3;
    int rc;

    struct chat_packet received_packet;
    struct client_command pack2;

    // Set up the listening socket
    rc = listen(listenfd, MAX_CONNECTIONS);
    DIE(rc < 0, "listen");


    // Add file descriptors to the poll set:
    poll_fds[0].fd = STDIN_FILENO; // Standard input for commands
    poll_fds[0].events = POLLIN;

    poll_fds[1].fd = listenfd;  // TCP listening socket
    poll_fds[1].events = POLLIN;

    poll_fds[2].fd = listen_udp; // UDP listening socket
    poll_fds[2].events = POLLIN; 

    int newsockfd2;
    while (1) {

        rc = poll(poll_fds, num_clients, -1);
        DIE(rc < 0, "poll");

        for (int i = 0; i < num_clients; i++) {
            if(poll_fds[i].fd == STDIN_FILENO){
                // Handling user input from stdin
                struct timeval tv; 
                fd_set readfds;
                int retval;

                tv.tv_sec = 0;
                tv.tv_usec = 100;

                //setting a time period for the user to respond

                FD_ZERO(&readfds);
                FD_SET(0, &readfds);

                // don't care about writefds and exceptfds:
                retval = select(1, &readfds, NULL, NULL, &tv);
                if (retval == -1) 
                    perror("select()");
                else if (retval > 0)
                {

                    char buf[1500];
                    cin>>buf;
                    
                    if(!strcmp(buf, "exit")){
                        return;  // Exit on "exit" command
                    } else{
                        continue; // invalid command
                    }
                }   
                else
                    continue;
                
         
            } else if (poll_fds[i].revents & POLLIN) {
                if (poll_fds[i].fd == listenfd) {
                    // a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
                    // pe care serverul o accepta
                    struct sockaddr_in cli_addr;
                    socklen_t cli_len = sizeof(cli_addr);
                    int newsockfd =
                            accept(listenfd, (struct sockaddr *) &cli_addr, &cli_len);
                    DIE(newsockfd < 0, "accept");

                    // Receive client ID and check if it is already connected
                    char buf[1500];
                    rc = recv(newsockfd, buf, sizeof(buf), 0);
                    if (rc < 0){
                        perror("recv");
                        close(newsockfd);
                        continue;
                    }


                    size_t ok = 0;
                    if(clientid_to_topic.find(buf) !=clientid_to_topic.end()){
                        std :: cout << "Client " << buf << " already connected.\n";
                        rc = send(newsockfd, &ok, sizeof(size_t), 0);
                        DIE(rc < 0, "send");
                        continue;
                    }


                    
                    // Add the new socket to the poll set

                    poll_fds[num_clients].fd = newsockfd;
                    poll_fds[num_clients].events = POLLIN;
                    num_clients++;
                    newsockfd2 = newsockfd;
                    socket_to_id[newsockfd] = buf;
                    id_to_socket[buf] = newsockfd;

                    clientid_to_topic[buf] = vector<string>(); // Initialize client's topics
                    std::cout << "New client " << buf << " connected from "<< inet_ntoa(cli_addr.sin_addr) <<":"<< ntohs(cli_addr.sin_port)<<"  "<<newsockfd<<".\n";
                    ok = 1;
                    rc = send(newsockfd, &ok, sizeof(size_t), 0);
                    DIE(rc < 0, "send");

                } else if (poll_fds[i].fd == listen_udp){
                    // Handle incoming UDP message and process it for subscribers


                    struct udp_packet udp_pkt;
                    struct sockaddr_in server_address;
                    socklen_t slen = sizeof(server_address);
                    int g = recvfrom(poll_fds[i].fd, &udp_pkt , sizeof(udp_pkt), 0, (sockaddr*)&server_address, &slen);
                    
                    //build the string to be parsed
                    string statement;
                    //converts the ip adress in network byte order

                    statement+= inet_ntoa(server_address.sin_addr)+string(":");
                    // the same for port
                    statement+=std::to_string(ntohs(server_address.sin_port))+string(" - ");
                    //ensure the a message is sent one time
                    set<string> isContaining = set<string>();
                    


                    for(auto it3 = clientid_to_topic.begin() ; it3 != clientid_to_topic.end(); it3++){
                        
                        for(vector<string>::iterator it4 = it3->second.begin() ; it4 != it3->second.end(); it4++){
                            ///ensure that the udp_pachet is sent only one time
                            if(!isContaining.count(udp_pkt.topic)){

                                //check if the value matches or respects the wildcard rule
                                if(!strcmp(it4->c_str(), udp_pkt.topic) || match_elements(it4->c_str(), udp_pkt.topic)==2){
                                    
                                    int getSocket = id_to_socket[it3->first];
                                    int rc = send_all(getSocket, &udp_pkt, sizeof(udp_pkt));
                                    DIE(rc < 0, "send");
                                    isContaining.insert(udp_pkt.topic);
                                    
                                }
                            }
                        }
                    }
                    //cout<<"not entered";
                    //cout<<"not entered!!!"<<clientid_to_topic.size()<<".\n";


                }else {
                                        
                    // Receive data from a client, 
                    // server process the command


                    int rc = recv_all(poll_fds[i].fd, &pack2,
                                      sizeof(pack2));

                    if (rc == 0) {
                        // Handle client disconnection
                        unordered_map<int, string> :: iterator it = socket_to_id.find(poll_fds[i].fd);
                            
                        if(it == socket_to_id.end()){
                            perror("does not exist");
                        } else{
                            string id2 = it->second;
                            clientid_to_topic.erase(id2);
                            socket_to_id.erase(poll_fds[i].fd);
                            id_to_socket.erase(id2);

                            std::cout << "Client "<<  id2 << " disconnected.\n";

                            // Remove closed socket from poll set
                            for (int j = i; j < num_clients - 1; j++) {
                                    poll_fds[j] = poll_fds[j + 1];
                            }
                            poll_fds[i].revents = 0;

                            close(poll_fds[i].fd);


                            num_clients--;
                            i=-1;
                            continue;
                            

                        }
                        

                    } else {
                        if(!strcmp(pack2.command, "exit")){

                            // Handle specific client commands like "exit", "subscribe", "unsubscribe"

                            unordered_map<int, string> :: iterator it = socket_to_id.find(poll_fds[i].fd);
                            
                            if(it == socket_to_id.end()){
                                perror("does not exist");
                            } else{

                                string id2 = it->second;
                                clientid_to_topic.erase(id2);
                                socket_to_id.erase(poll_fds[i].fd);
                                id_to_socket.erase(id2);

                                std::cout << "Client "<<id2 <<" disconnected.\n";
                                

                                // Remove closed socket from poll set
                                for (int j = i; j < num_clients - 1; j++) {
                                        poll_fds[j] = poll_fds[j + 1];
                                }
                                poll_fds[i].revents = 0;
                                close(poll_fds[i].fd);

                                num_clients--;

                            }


                        } else
                         if(!strcmp(pack2.command, "subscribe")){
                            // Handle "subscribe" command for a topic

                            unordered_map<int, string> :: iterator it = socket_to_id.find(poll_fds[i].fd);
                            unordered_map<string, vector<string>> :: iterator it2 = clientid_to_topic.find(it->second);

                            vector<string> topics = it2->second;
                            int pos = -1, pos2=-1;
                            //check if there is already a subscriber
                            for(string name:topics){
                                pos++;
                                if(!name.compare(pack2.content)){
                                  pos2=pos;
                                  break;
                                }
                            }


                            if(pos2!=-1){
                                std::cout << "Client "<< it->second << " is already subscribed to this topic.\n";
                                close(poll_fds[i].fd);
                                continue;
                            }
                          
                            clientid_to_topic[it->second].push_back(pack2.content);
                            topicsTotal.push_back(pack2.content);


                        } else if(!strcmp(pack2.command, "unsubscribe")){
                            // Handle "unsubscribe" command for a topic

                            unordered_map<int, string> :: iterator it = socket_to_id.find(poll_fds[i].fd);
                            unordered_map<string, vector<string>> :: iterator it2 = clientid_to_topic.find(it->second);

                            vector<string> topics = it2->second;
                            int pos = -1, pos2=-1;
                            for(string it3:topics){
                                pos++;
                                if(!it3.compare(pack2.content)){
                                  pos2=pos;
                                  break;
                                }
                            }

                            if(pos2==-1) {
                                std::cout << "Client "<< it->second << " is not subscribed to this topic.\n";
                                continue;
                            } else {
                                clientid_to_topic[it->second].erase(std::next(clientid_to_topic[it->second].begin(),pos2));
                            }
                            
                        }
                        
                        
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    if (argc != 2) {
        printf("\n Usage: %s <port>.\n", argv[0]);
        return 1;
    }

    // Parse the port number
    uint16_t port;
    int rc = sscanf(argv[1], "%hu", &port);
    DIE(rc != 1, "Given port is invalid");


    // Create a TCP socket for receiving connections
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(listenfd < 0, "socket");

    // Prepare server address structure
    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);


    // Set socket options to disable Nagle’s algorithm for low latency
    int on = 1;
    int result = setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY, (char*)&on, sizeof(int));
    if(result < 0){
        perror("setsockopt(TCP_NODELAY) failed");
    }
    
    // Enable socket reuse option for rapid re-runs
    int enable = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    

    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    

    // Bind server address to the TCP socket
    rc = bind(listenfd, (const struct sockaddr *) &serv_addr, sizeof(serv_addr));
    DIE(rc < 0, "bind");

    // Create a UDP socket for receiving connections
    int listen_udp = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(listen_udp < 0, "socket");
  

    // Bind server address to the UDP socket
    rc = bind(listen_udp, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(rc < 0, "bind");


    // Start the multi-client chat server handling both TCP and UDP connections
    run_chat_multi_server(listenfd, listen_udp);

    // Close the listening TCP socket
    close(listenfd);

    return 1;
}
