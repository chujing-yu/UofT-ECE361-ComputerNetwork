#ifndef PACKET_H
#define PACKET_H

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1100
#define DATA_SIZE 1000

typedef struct aPacket {
  unsigned int total_frag;
  unsigned int frag_no;
  unsigned int size;
  char *filename;
  char filedata[DATA_SIZE];
} Packet;

void packetToString(const Packet *p, char *str) {
  // Initialize string buffer
  memset(result, 0, BUF_SIZE);

  // turn packet into string
  int i = 0;
  sprintf(str, "%d:%d:%d:%s", p->total_frag, p->frag_no, p->size, p->filename);
  i = strlen(str);

  // Copy filedata
  int copySize = p->size < DATA_SIZE ? p->size : DATA_SIZE;
  memcpy(str + i, p->filedata, copySize);
}

void stringToPacket(const char *str, Packet *pa) {
  // split the string into parts based on ":"
  char *token = strtok((char *)str, ":");
  pa->total_frag = atoi(token);
  token = strtok(NULL, ":");
  pa->frag_no = atoi(token);
  token = strtok(NULL, ":");
  pa->size = atoi(token);
  token = strtok(NULL, ":");
  strncpy(pa->filename, token, BUF_SIZE);
  token = strtok(NULL, ":");
  memcpy(pa->filedata, token, pa->size);
}

void printPacket(Packet *packet) {
  printf("total_frag = %d,\n frag_no = %d, size = %d, filename = %s\n",
         packet->total_frag, packet->frag_no, packet->size, packet->filename);
  char data[DATA_SIZE + 1] = {0};
  memcpy(data, packet->filedata, DATA_SIZE);
  printf("%s\n", data);
}

#endif