#include "common.h"

#include <sys/socket.h>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
/*
    1.1: Rescrieți funcția de mai jos astfel încât ea să facă primirea
    a exact len octeți din buffer.
*/
int send_all(int sockfd, void *buffer, size_t len) {
  	size_t bytes_sent = 0;
  	size_t bytes_remaining = len;
  	char *buff = (char*)buffer;

  	while (bytes_remaining) {
		int rc = send(sockfd, buff + bytes_sent, bytes_remaining, 0);

        if (rc < 0) {
            exit(1);
        }
        
	  	if (bytes_sent == 0) {
			return bytes_sent;
        }

	  	bytes_sent += rc;
	  	bytes_remaining -= rc;
	}
  
  	return bytes_sent;
}

// Primesc intregul mesaj de pe socket-ul sockfd
int recv_all(int sockfd, void *buffer, size_t len) {
	size_t bytes_received = 0;
	size_t bytes_remaining = len;
	char *buff = (char*)buffer;

	while (bytes_remaining) {
		int rc = recv(sockfd, buff + bytes_received, bytes_remaining, 0);

        if (rc < 0) {
            exit(1);
        }

		if (rc == 0) {
			return bytes_received;
        }
        
		bytes_received += rc;
		bytes_remaining -= rc;
	}

  return bytes_received;
}

int send_vector(int sockfd, std::vector<std::string> element){
    size_t sizeVec = element.size();
    send(sockfd, &sizeVec, sizeof(size_t), 0);
    for(const std::string& vec: element){
       int size_string = vec.size();
       send(sockfd, &size_string, sizeof(size_t), 0);
       char elementChar[size_string];
       strcpy(elementChar, vec.c_str());
       send(sockfd, elementChar, sizeof(elementChar), 0);
    }
    return sizeVec;


}

std::vector<std::string> recv_vector2(int sockfd, std::vector<std::string> element){
    element.clear();
    size_t sizeVec;
    recv(sockfd, &sizeVec, sizeof(size_t), 0);
    for(int i=0;i<sizeVec;i++){
        int size_string;
        recv(sockfd, &size_string, sizeof(size_t), 0);
        char elementChar[size_string];
        recv(sockfd, elementChar, sizeof(elementChar), 0);
        element.push_back(elementChar);
    }
    return element;


}

