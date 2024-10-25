/*
 * Protocoale de comunicatii
 * Laborator 7 - TCP
 * Echo Server
 * server.c
 */

#include <arpa/inet.h>
#include <errno.h>
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
#include<vector>
#include<unordered_map>
#include "common.h"
#include "helpers.h"

#define MAX_CONNECTIONS 32
#define FULLMSG_SIZE 2000
using namespace std;
unordered_map<string, vector<string> > clientid_to_topic;
unordered_map<int, string> socket_to_id;
unordered_map<string, int> id_to_socket;
/// refactor here need to do a class


// Primeste date de pe connfd1 si trimite mesajul receptionat pe connfd2
int receive_and_send(int connfd1, int connfd2, size_t len) {
    int bytes_received;
    char buffer[len];

    // Primim exact len octeti de la connfd1
    bytes_received = recv_all(connfd1, buffer, len);
    // S-a inchis conexiunea
    if (bytes_received == 0) {
        return 0;
    }
    DIE(bytes_received < 0, "recv");

    // Trimitem mesajul catre connfd2
    int rc = send_all(connfd2, buffer, len);
    if (rc <= 0) {
        perror("send_all");
        return -1;
    }

    return bytes_received;
}

void run_chat_server(int listenfd) {
    struct sockaddr_in client_addr1;
    struct sockaddr_in client_addr2;
    socklen_t clen1 = sizeof(client_addr1);
    socklen_t clen2 = sizeof(client_addr2);

    int connfd1 = -1;
    int connfd2 = -1;
    int rc;

    // Setam socket-ul listenfd pentru ascultare
    rc = listen(listenfd, 2);
    DIE(rc < 0, "listen");

    // Acceptam doua conexiuni
    printf("Astept conectarea primului client...\n");
    connfd1 = accept(listenfd, (struct sockaddr *) &client_addr1, &clen1);
    DIE(connfd1 < 0, "accept");

    printf("Astept connectarea clientului 2...\n");

    connfd2 = accept(listenfd, (struct sockaddr *) &client_addr2, &clen2);
    DIE(connfd2 < 0, "accept");

    while (1) {
        // Primim de la primul client, trimitem catre al 2lea
        printf("Primesc de la 1 si trimit catre 2...\n");
        int rc = receive_and_send(connfd1, connfd2, sizeof(struct chat_packet));
        if (rc <= 0) {
            break;
        }

        rc = receive_and_send(connfd2, connfd1, sizeof(struct chat_packet));
        if (rc <= 0) {
            break;
        }
    }

    // Inchidem conexiunile si socketii creati
    close(connfd1);
    close(connfd2);
}

void run_chat_multi_server(int listenfd, int listen_udp) {

    struct pollfd poll_fds[MAX_CONNECTIONS];
    int num_clients = 3;
    int rc;

    struct chat_packet received_packet;
    struct client_command pack2;

    // Setam socket-ul listenfd pentru ascultare
    rc = listen(listenfd, MAX_CONNECTIONS);
    DIE(rc < 0, "listen");

    // se adauga noul file descriptor (socketul pe care se asculta conexiuni) in
    // multimea read_fds
    poll_fds[0].fd = STDIN_FILENO;
    poll_fds[0].events = POLLIN;

    poll_fds[1].fd = listenfd;
    poll_fds[1].events = POLLIN;

    poll_fds[2].fd = listen_udp;
    poll_fds[2].events = POLLIN;   

    while (1) {

        
        

        rc = poll(poll_fds, num_clients, -1);
        DIE(rc < 0, "poll");

        for (int i = 0; i < num_clients; i++) {

            if(poll_fds[i].fd == STDIN_FILENO){
                struct timeval tv; 
                fd_set readfds;
                int retval;

                tv.tv_sec = 1;
                tv.tv_usec = 0;

                FD_ZERO(&readfds);
                FD_SET(0, &readfds);

                // don't care about writefds and exceptfds:
                retval = select(1, &readfds, NULL, NULL, &tv);
                if (retval == -1) 
                    perror("select()");
                else if (retval > 0)
                {
                    char buf[1500];
                    fgets(buf, sizeof(buf), stdin);
                    
                    if(!strncmp(buf, "exit", 4)){
                    return;
                    } else{
                    continue;
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
                    char buf[1500];
                    recv(newsockfd, buf, sizeof(buf), 0);

                    if(clientid_to_topic.find(buf) !=clientid_to_topic.end()){
                        std :: cout << "Client " << buf << " already connected.\n";
                        continue;
                    }

                    clientid_to_topic[buf] = vector<string>();

                    
                    // se adauga noul socket intors de accept() la multimea descriptorilor
                    // de citire
                    poll_fds[num_clients].fd = newsockfd;
                    poll_fds[num_clients].events = POLLIN;
                    num_clients++;

                    socket_to_id[newsockfd] = buf;
                    id_to_socket[buf] = newsockfd;
                    std::cout << "New client " << buf << " connected from "<< inet_ntoa(cli_addr.sin_addr) <<":"<< ntohs(cli_addr.sin_port)<<".\n";
                } else if (poll_fds[i].fd == listen_udp){

                    struct udp_packet udp_pkt;
                    struct sockaddr_in server_address;
                    socklen_t slen = sizeof(server_address);
                    int g = recvfrom(poll_fds[i].fd, &udp_pkt , sizeof(udp_packet), 0, (struct sockaddr *)&server_address, &slen);
                

                    string statement;
                    statement+= inet_ntoa(server_address.sin_addr)+string(":");
                    statement+=std::to_string(ntohs(server_address.sin_port))+string(" - ");
                    statement = buildStatement(statement, udp_pkt);
    
                    std::cout<<statement<<"\n";

                    for(auto it3 = clientid_to_topic.begin() ; it3 != clientid_to_topic.end(); it3++){
                        for(vector<string>::iterator it4 = it3->second.begin() ; it4 != it3->second.end(); it4++){
                            if(statement.find(it4->c_str()) != string::npos){

                                int getSocket = id_to_socket[it3->first];
                                size_t statementLen = statement.length();
                                int rc2 = send(getSocket, &statementLen, sizeof(size_t), 0);
                                DIE(rc2 < 0, "send");
                                char* statementChar = new char[statementLen];
                                strcpy(statementChar, statement.c_str());
                                send_all(getSocket, statementChar, sizeof(statementChar));
                            }
                        }
                    }


                }else {
                    // s-au primit date pe unul din socketii de client,
                    // asa ca serverul trebuie sa le receptioneze
                    int rc = recv_all(poll_fds[i].fd, &pack2,
                                      sizeof(pack2));
                    DIE(rc < 0, "recv");
                    if (rc == 0) {
                        unordered_map<int, string> :: iterator it = socket_to_id.find(poll_fds[i].fd);
                            
                        if(it == socket_to_id.end()){
                            perror("does not exist");
                        } else{
                            string id2 = it->second;
                            clientid_to_topic.erase(id2);
                            socket_to_id.erase(poll_fds[i].fd);
                            id_to_socket.erase(id2);

                            std::cout << "Client "<<  id2 << " disconnected.\n";

                            // se scoate din multimea de citire socketul inchis
                            for (int j = i; j < num_clients - 1; j++) {
                                    poll_fds[j] = poll_fds[j + 1];
                            }

                            num_clients--;
                            

                        }
                        

                    } else {
                        
                        if(!strcmp(pack2.command, "exit")){
                            unordered_map<int, string> :: iterator it = socket_to_id.find(poll_fds[i].fd);
                            
                            if(it == socket_to_id.end()){
                                perror("does not exist");
                            } else{

                                string id2 = it->second;
                                clientid_to_topic.erase(id2);
                                socket_to_id.erase(poll_fds[i].fd);
                                id_to_socket.erase(id2);

                                std::cout << "Client "<<id2 <<" disconnected.\n";
                                close(poll_fds[i].fd);

                                // se scoate din multimea de citire socketul inchis
                                for (int j = i; j < num_clients - 1; j++) {
                                        poll_fds[j] = poll_fds[j + 1];
                                }

                                num_clients--;

                            }


                        } else if(!strcmp(pack2.command, "subscribe")){
                            unordered_map<int, string> :: iterator it = socket_to_id.find(poll_fds[i].fd);
                            cout<<it->second<<".\n";
                            unordered_map<string, vector<string> > :: iterator it2 = clientid_to_topic.find(it->second);

                            vector<string> topics = it2->second;
                            int pos = -1, pos2=-1;
                            for(vector<string>::iterator it3 = topics.begin() ; it3 != topics.end(); it3++){
                                pos++;
                                string name = it3->c_str();
                                if(!name.compare(pack2.content)){
                                  pos2=pos;
                                  break;
                                }
                            }
                            if(pos2!=-1){
                                std::cout << "Client "<< it->second << " is already subscribed to this topic.\n";
                                continue;
                            }
                            
                            topics.push_back(pack2.content);
                            clientid_to_topic[it->second] = topics;
                            


                        } else if(!strcmp(pack2.command, "unsubscribe")){
                            unordered_map<int, string> :: iterator it = socket_to_id.find(poll_fds[i].fd);
                            unordered_map<string, vector<string> > :: iterator it2 = clientid_to_topic.find(it->second);

                            vector<string> topics = it2->second;
                            int pos = -1, pos2=-1;
                            for(vector<string>::iterator it3 = topics.begin() ; it3 != topics.end(); it3++){
                                pos++;
                                string name = it3->c_str();
                                if(!name.compare(pack2.content)){
                                  pos2=pos;
                                  break;
                                }
                            }

                            if(pos2==-1) {
                                std::cout << "Client "<< it->second << " is not subscribed to this topic.\n";
                                continue;
                            } else {
                                topics.erase(topics.begin()+pos2);
                            }

                            clientid_to_topic[it->second] = topics;

                            
                        }
                        
                        /* 2.1: Trimite mesajul catre toti ceilalti clienti */
                        /*for (int j = 0; j < num_clients; j++) {
                            if (poll_fds[j].fd != listenfd && poll_fds[j].fd != STDIN_FILENO &&
                            poll_fds[j].fd != listen_udp && poll_fds[j].fd != poll_fds[i].fd) {
                                
                                int sent_bytes = send_all(poll_fds[j].fd, &pack2, sizeof(pack2));
                                DIE(sent_bytes < 0, "send");
                            }
                        }*/
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

    // Parsam port-ul ca un numar
    uint16_t port;
    int rc = sscanf(argv[1], "%hu", &port);
    DIE(rc != 1, "Given port is invalid");


    // Obtinem un socket TCP pentru receptionarea conexiunilor
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(listenfd < 0, "socket");

    // Completăm in serv_addr adresa serverului, familia de adrese si portul
    // pentru conectare
    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);

    // Facem adresa socket-ului reutilizabila, ca sa nu primim eroare in caz ca
    // rulam de 2 ori rapid
    int enable = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    int on = 1;
    int result = setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY, (char*)&on, sizeof(int));
    if(result < 0){
        perror("setsockopt(TCP_NODELAY) failed");
    }

    

    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    

    // Asociem adresa serverului cu socketul creat folosind bind
    rc = bind(listenfd, (const struct sockaddr *) &serv_addr, sizeof(serv_addr));
    DIE(rc < 0, "bind");

    int listen_udp = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(listen_udp < 0, "socket");
    
    int on2 = 1;
    int result2 = setsockopt(listen_udp, IPPROTO_UDP, TCP_NODELAY, (char*)&on2, sizeof(int));
    if(result2 < 0){
        perror("setsockopt(TCP_NODELAY) failed");
    }
    
    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    
    rc = bind(listen_udp, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(rc < 0, "bind");



    run_chat_multi_server(listenfd, listen_udp);

    // Inchidem listenfd
    close(listenfd);

    return 1;
}
