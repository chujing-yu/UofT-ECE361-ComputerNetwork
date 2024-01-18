#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "packet.h"

#define RetransMax 10 

// slice the file into fragments and wait for ACK
void send_file(char *filename, int sockfd, struct sockaddr_in servaddr) {
  // Open file
  FILE *fp;
  if ((fp = fopen(filename, "r")) == NULL) {
    printf("Can't open input file %s\n", filename);
    exit(1);
  }

  // The total number of fragments
  fseek(fp, 0, SEEK_END);
  int total_frag = ftell(fp) / DATA_SIZE + 1;
  printf("Total packets(frags) number: %d\n", total_frag);
  rewind(fp);

  // Slice the file into packets
  char rec_buf[BUF_SIZE];  // Buffer for receiving packets
  char **packets =
      malloc(sizeof(char *) * total_frag);  // Stores packets for retransmitting
  for (int packet_num = 1; packet_num <= total_frag; ++packet_num) {
    Packet packet;
    memset(packet.filedata, 0, sizeof(char) * (DATA_SIZE));
    fread((void *)packet.filedata, sizeof(char), DATA_SIZE, fp);

    // Update the members of packet
    packet.total_frag = total_frag;
    packet.frag_no = packet_num;
    packet.filename = filename;
    if (packet_num != total_frag) {
      packet.size = DATA_SIZE;
    } else {
      fseek(fp, 0, SEEK_END);
      packet.size = (ftell(fp) - 1) % DATA_SIZE + 1;
    }

    packets[packet_num - 1] = malloc(BUF_SIZE * sizeof(char));
    packetToString(&packet, packets[packet_num - 1]);
  }

  // Send packets
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                 sizeof(timeout)) < 0) {
    printf("error in setsockopt\n");
  }
  int timesent = 0;  // retransmission times
  socklen_t serv_addr_size = sizeof(servaddr);

  Packet ack_packet;  // ACKs
  ack_packet.filename = (char *)malloc(BUF_SIZE * sizeof(char));

  // Send frag and receive ACK, until all fragments are sent successfully and
  // ACKs are received.
  for (int packet_num = 1; packet_num <= total_frag; ++packet_num) {
    int n;
    timesent++;

    // Send frag
    if ((n = sendto(sockfd, packets[packet_num - 1], BUF_SIZE, 0,
                    (struct sockaddr *)&servaddr, sizeof(servaddr))) == -1) {
      printf("sendto error for packet #%d\n", packet_num);
      exit(1);
    }
    

    // Receive acknowledgements
    memset(rec_buf, 0, sizeof(char) * BUF_SIZE);
    if ((n = recvfrom(sockfd, rec_buf, BUF_SIZE, 0,
                      (struct sockaddr *)&servaddr, &serv_addr_size)) == -1) {
      // Resend if do not receive an ack within timeout
      printf(
          "Timeout or recvfrom error for ACK packet #%d, resending attempt "
          "#%d...\n",
          packet_num--, timesent);
      if (timesent < RetransMax) {
        continue;
      } else {
        printf("Having resent too many times. Stop the transfer.\n");
        exit(1);
      }
    }

    stringToPacket(rec_buf, &ack_packet);

    // Check ACK
    if (strcmp(ack_packet.filename, filename) == 0 &&
        ack_packet.frag_no == packet_num &&
        strcmp(ack_packet.filedata, "ACK") == 0) {
      timesent = 0;
      continue;
    }

    // Resend if ACK is not the correct one I am looking for
    printf("ACK packet #%d not received, resending attempt #%d...\n",
           packet_num, timesent);
    --packet_num;
  }

  // Free memory
  for (int packet_num = 1; packet_num <= total_frag; ++packet_num) {
    free(packets[packet_num - 1]);
  }
  free(packets);
  free(ack_packet.filename);
}

int main(int argc, char const *argv[]) {
  int sockfd;
  char buffer[BUF_SIZE] = {0};
  char *file_str = "ftp";
  struct sockaddr_in servaddr;
  int PORT = atoi(argv[2]);

  // Creating socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset((char *)&servaddr, 0, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(argv[1]);
  servaddr.sin_port = htons(PORT);

  printf("Input a message follow the format \"ftp <file name>:\"\n");
  char ftp[10];
  char fileName[100];
  int flag = 0;  // 0: not exist; 1: exist
  scanf("%s", ftp);
  scanf("%s", fileName);
  FILE *file = fopen(fileName, "r");
  if (file == NULL) {
    printf("THe file doesn't exist.\n");
    exit(1);
  } else {
    flag = 1;
    fclose(file);
  }

  int n;
  // send "ftp" message to server
  clock_t start, end;  // timer variables,to calculate turn-around time in Sec2
  start = clock();
  sendto(sockfd, "ftp", strlen("ftp"), 0, (struct sockaddr *)&servaddr,
         sizeof(servaddr));
  memset(buffer, 0, BUF_SIZE);  // clean the buffer
  socklen_t serv_addr_size = sizeof(servaddr);
  n = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&servaddr,
               &serv_addr_size);
  end = clock();
  double time = (double)(end - start) / CLOCKS_PER_SEC;
  printf("round-trip time:%f\n", time);

  buffer[n] = '\0';
  printf("%s", buffer);

  if (strcmp(buffer, "yes") == 0) {  // be replied with "yes"
    printf("A file transfer can start\n");
  } else {
    printf("A file transfer cannot start\n");
    exit(EXIT_FAILURE);
  }

  // Begin sending file and check for acknowledgements
  send_file(fileName, sockfd, servaddr);

  // Sending Completed
  close(sockfd);
  printf("Deliver finished. The file has been transfered.\n");
  return 0;
}
