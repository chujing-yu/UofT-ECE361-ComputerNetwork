// server side implement of the UDP client-server model
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// #define PORT  8080
#define MAXLINE 1024

// Driver code
int main(int argc, char *argv[]) {
  int sockfd;
  char buffer[MAXLINE];
  char *true_reply = "yes\n";
  char *false_reply = "no\n";
  struct sockaddr_in servaddr, cliaddr;

  int PORT = atoi(argv[1]);

  // Creating socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  // Filling server information./
  servaddr.sin_family = AF_INET;  // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  int len, n;

  len = sizeof(cliaddr);
  n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
               (struct sockaddr *)&cliaddr, &len);
  buffer[n] = '\0';
  if (strcmp(buffer, "ftp") == 0) {
    sendto(sockfd, (const char *)true_reply, strlen(true_reply), MSG_CONFIRM,
           (const struct sockaddr *)&cliaddr, len);
  } else {
    sendto(sockfd, (const char *)false_reply, strlen(false_reply), MSG_CONFIRM,
           (const struct sockaddr *)&cliaddr, len);
  }

  // printf("server has finished.\n");

  return 0;
}