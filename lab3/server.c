#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "packet.h"

int main(int argc, char const *argv[]) {
  int sockfd;
  char buffer[BUF_SIZE] = {0};
  char *true_reply = "yes";
  char *false_reply = "no";
  struct sockaddr_in servaddr, cliaddr;

  int PORT = atoi(argv[1]);

  // open socket (DGRAM)
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }


  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  // Filling server information.
  servaddr.sin_family = AF_INET;  // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  // bind to socket
  if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  int len, n;
  len = sizeof(cliaddr);

  // recvfrom "ftp" from the client and store info in cliaddr so as to send back
  // later
  n = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
  buffer[n] = '\0';

  // send "yes" back to client implying file transmission can start now
  if (strcmp(buffer, "ftp") == 0) {
    sendto(sockfd, (const char *)true_reply, strlen(true_reply), MSG_CONFIRM,
           (const struct sockaddr *)&cliaddr, len);
  } else {
    sendto(sockfd, (const char *)false_reply, strlen(false_reply), MSG_CONFIRM,
           (const struct sockaddr *)&cliaddr, len);
  }

  Packet packet;
  packet.filename = (char *)malloc(BUF_SIZE);  // allocate space for filename
  char filename[BUF_SIZE] = {0};
  FILE *pFile = NULL;
  bool *fragRecv = NULL;  // show if we have received flag

  bool firstPacketReceived = false;  // Add this flag

  // Receive packets continuously, write the information of packets into files
  // Then send message to client, until receive the last packet, which means the
  // complement of the file transfer
  while (1) {
    if (recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&cliaddr,
                 &len) == -1) {
      printf("recvfrom error\n");
      exit(1);
    }
    stringToPacket(buffer, &packet);  // turn the received string into packet.
                                      // now we have a received packet

    if (!firstPacketReceived) {
      firstPacketReceived = true;
    } else {
      // ensure that memory is only allocated when a packet is received for the
      // first time
      if (pFile == NULL) {              // if it is the first frag
        // Check whether the file stream has been opened for writing data
        strcpy(filename, packet.filename);
        while (access(filename, F_OK) == 0) {
          char *pSuffix = strrchr(filename, '.');
          char suffix[BUF_SIZE + 1] = {0};
          strncpy(suffix, pSuffix, BUF_SIZE - 1);
          *pSuffix = '\0';
          strcat(filename, "(2)");
          strcat(filename, suffix);
        }
        pFile = fopen(filename, "w");
      }

      // ensure that memory is only allocated when a packet is received for the
      // first time
      if (fragRecv == NULL) {
        fragRecv = (bool *)malloc(packet.total_frag * sizeof(fragRecv));
        for (int i = 0; i < packet.total_frag; i++) {
          fragRecv[i] = false;
        }
      }
      if (!fragRecv[packet.frag_no]) {
        // write the filedata into pFile
        int numbyte = fwrite(packet.filedata, sizeof(char), packet.size, pFile);
        if (numbyte != packet.size) {
          printf("fwrite error\n");
          exit(1);
        }
        fragRecv[packet.frag_no] = true;
      }
    }

    strcpy(packet.filedata, "ACK");  // ACK:receive successfully
    // send ACK back to the client
    packetToString(&packet, buffer);
    if ((sendto(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&cliaddr,
                sizeof(cliaddr))) == -1) {
      printf("sendto error\n");
      exit(1);
    }

    // Check if the transfer has completed (the last packet)
    if (packet.frag_no == packet.total_frag) {
      printf("File %s transfer completed\n", filename);
      break;
    }
  }

  close(sockfd);
  fclose(pFile);
  free(fragRecv);
  free(packet.filename);
  return 0;
}

