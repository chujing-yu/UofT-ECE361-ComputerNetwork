// Client side implementation of UDP client-server model
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// #define PORT  8080
#define MAXLINE 1024

int main(int argc, char *argv[]) {
  int sockfd;
  char buffer[MAXLINE];
  char *true_reply = "yes";
  char *false_reply = "no";
  char *file_str = "ftp";
  char *no_file = "notExist";
  struct sockaddr_in servaddr;

  int PORT = atoi(argv[2]);
 
  

  // Creating socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));

  // Filling server information
  servaddr.sin_family = AF_INET;  // IPv4
  // servaddr.sin_addr.s_addr=INADDR_ANY;
  //memcpy(&servaddr.sin_addr, hp->h_addr, hp->h_length);
  servaddr.sin_addr.s_addr=inet_addr(argv[1]);
  servaddr.sin_port = htons(PORT);

  // Send message about file
  int n, len;

  printf("Input a message follow the format \"ftp <file name>:\"\n");
  char ftp[10];
  char fileName[100];
  // char* filename;
  int flag = 0;  // 0: not exist; 1: exist
  scanf("%s", ftp);
  scanf("%s", fileName);
  FILE *file = fopen(fileName, "r");
  if (file == NULL) {
    printf("THe file doesn't exist.\n");
    flag = 0;
  } else {
    flag = 1;
    fclose(file);
  }
  
  // printf("%d\n", flag);

  if (flag) {
    sendto(sockfd, (const char *)file_str, strlen(file_str), MSG_CONFIRM,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));
  } else {
    // printf("1\n");
    sendto(sockfd, (const char *)no_file, strlen(no_file), MSG_CONFIRM,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));
  }

  // Receive message from the server
  n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_CONFIRM,
               (struct sockaddr *)&servaddr, &len);
  buffer[n] = '\0';
  printf("%s", buffer);
  if (strcmp(buffer, "yes\n") == 0) {
    printf("A file transfer can start.\n");
  } else {
    // printf("2\n");
    exit(EXIT_FAILURE);
  }
  // printf("deliver finished.\n");
  return 0;
}
