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
  // Initialize the string buffer
  memset(str, 0, BUF_SIZE);

  // Copy the structured data to the string, excluding the binary data
  int cursor = 0;
  cursor += sprintf(str, "%d:%d:%d:%s:", p->total_frag, p->frag_no, p->size, p->filename);

  // Copy the binary data
  memcpy(str + cursor, p->filedata, p->size);
}


void stringToPacket(const char *buf, Packet *pkt) {
  char *temp[4];
  int colon_index[4];
  int index = 0;
  // find the position of colons
  for (int i = 0; i < BUF_SIZE; i++) {
    if (buf[i] == ':') {
      colon_index[index] = i;
      index++;
      if (index > 3) {
        break;
      } else
        continue;
    } else
      continue;
  }

  // copy
  int seg_start = 0;
  for (int i = 0; i < 4; i++) {
    int seg_len = colon_index[i] - seg_start;
    temp[i] = (char *)malloc(sizeof(char) * seg_len);
    memcpy(temp[i], buf + seg_start, seg_len);
    seg_start = colon_index[i] + 1;
  }
  pkt->total_frag = atoi(temp[0]);
  pkt->frag_no = atoi(temp[1]);
  pkt->size = atoi(temp[2]);
  pkt->filename = temp[3];
  for (int i = 0; i < 3; i++) free(temp[i]);

  memcpy(&(pkt->filedata), buf + seg_start, pkt->size);
}

void printPacket(Packet *packet) {
  printf("total_frag = %d,\n frag_no = %d, size = %d, filename = %s\n",
         packet->total_frag, packet->frag_no, packet->size, packet->filename);
  char data[DATA_SIZE + 1] = {0};
  memcpy(data, packet->filedata, DATA_SIZE);
  printf("%s\n", data);
}
#endif


