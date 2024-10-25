#ifndef __COMMON_H__
#define __COMMON_H__

#include <stddef.h>
#include <stdint.h>
#include<vector>
#include<unordered_map>

int send_all(int sockfd, void *buff, size_t len);

int recv_all(int sockfd, void *buff, size_t len);

/* Dimensiunea maxima a mesajului */
#define MSG_MAXSIZE 1024
#define CONTENT_MAXSIZE 1500
#define TOPIC_MAXSIZE 50
#define COMMAND_SIZE 30
struct chat_packet {
    uint16_t len;
    char message[MSG_MAXSIZE + 1];
};

struct client_command{
  struct chat_packet header;
  char command[COMMAND_SIZE + 1];
  char content[CONTENT_MAXSIZE + 1];
};

struct udp_client {
  char topic[TOPIC_MAXSIZE+1];
  uint8_t data_type;
  char content[CONTENT_MAXSIZE+1];
};

struct udp_packet {
  char topic[TOPIC_MAXSIZE];
  uint8_t data_type;
  char value[CONTENT_MAXSIZE+1];
};

struct subscriber_client {
  char topic[TOPIC_MAXSIZE+1];
  char command[11];
  char id[11];
  int wildcard;
};

#endif
