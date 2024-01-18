#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "ClientList.h"
#include "SessionList.h"
#include "global.h"
#include "packet.h"

//  #define IP "192.168.111.128" //virtual machine's ip
//  #define PORT 8080

// long cfds[10] = {-1}; 
// int n_client = 0;
//  Enforce synchronization
pthread_mutex_t sessionList_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t userLoggedin_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sessionCnt_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t userConnectedCnt_mutex = PTHREAD_MUTEX_INITIALIZER;

// global variables
ClientList *clientList = NULL;
SessionList *sessionList = NULL;

void *ReceiveClient(void *arg)
{
  char buffer[BUF_SIZE];
  char source[MAX_NAME];
  /*Packet* packet=(Packet*)malloc(sizeof(Packet));
  Packet* pkt_sent=(Packet*)malloc(sizeof(Packet));*/
  Packet packet;
  Packet pkt_sent;
  CoClient *newClient = (CoClient *)arg;
  bool toSent = false;
  bool toExit = false;
  bool loggedIn = false;
  // Client* newClient=arg;
  while (1)
  {
    memset(buffer, 0, (BUF_SIZE) * sizeof(char));
    //memset(source, 0, (MAX_NAME) * sizeof(char));
    memset(&packet, 0, sizeof(Packet));
    memset(&pkt_sent, 0, sizeof(Packet));
    int bytesRecvd;
    int bytesSent;
    if ((bytesRecvd = recv(newClient->sockfd, buffer, BUF_SIZE - 1, 0)) == -1)
    {
      perror("error recv\n");
      exit(1);
    }

    if (bytesRecvd == 0)
      toExit = true;
    buffer[bytesRecvd] = '\0';

    toSent = false;
    printf("Message received: %s \n", buffer);
    stringToPacket(buffer, &packet);
    printf("data: %s \n", packet.data);
    //printf("Message received: %s \n", buffer);

    if (packet.type == EXIT)
    {
      toExit = true;
    }
    if (loggedIn == false)
    {
      if (packet.type == LOGIN)
      {
        //printf("recevied type=login.\n");
        // check the name and password
        strcpy(newClient->clientID, packet.source);
        strcpy(newClient->passwords, packet.data);

        //printf("%s %s\n", newClient->clientID, newClient->passwords);
        //printf("prepare to index.\n");
        int index = FindClient(clientList, newClient->clientID);
        //printf("%d\n", index);
        //printf("didnot stuck in get index.\n");
        if (index == -1)
        {
          printf("No Find the client ID.\n");
          strcpy(pkt_sent.data, "No Find the client ID.\n");
          pkt_sent.type = LO_NAK;
          toSent = true;
        }
        else
        {
          if (CheckPassword(clientList, index, newClient->passwords) == false)
          {
            printf("Wrong password.\n");
            strcpy(pkt_sent.data, "Wrong password.\n");
            pkt_sent.type = LO_NAK;
            toSent = true;
          }
          else
          {

            printf("the client already joined.\n");
            // if the client already joined
            Client *temp = clientList->head;
            for (int i = 0; i < index; i++)
            {
              temp = temp->next;
            }
            if (temp->isLog == true)
            {
              printf("The client has already logged in.\n");
              strcpy(pkt_sent.data, "The client has already logged in.\n");
              pkt_sent.type = LO_NAK;
              toSent = true;
            }
            else
            {
              printf("Prepare to send login_ack.\n");
              temp->isLog = true;
              toSent = true;
              pkt_sent.type = LO_ACK;
              strcpy(pkt_sent.data, "Successfully logged in.\n");
              strcpy(source, packet.source);
              loggedIn = true;

              pthread_mutex_lock(&userLoggedin_mutex);
              pthread_mutex_unlock(&userLoggedin_mutex);
            }
          }
        }
      }
    }
    else
    {
      if (packet.type == JOIN)
      {
        char sessionId[MAX_DATA];
        strcpy(sessionId, packet.data);

        int index = FindSession(sessionList, sessionId);
        if (index == -1)
        {
          pkt_sent.type = JN_NAK;
          toSent = true;
          strcpy(pkt_sent.data, "The session doesn't exit.\n");
        }
        else if (CheckClient(newClient, sessionId) == true)
        {
          pkt_sent.type = JN_NAK;
          toSent = true;
          strcpy(pkt_sent.data, "you have already joined the session.\n");
        }
        else
        {
          pkt_sent.type = JN_ACK;
          toSent = true;
          strcpy(pkt_sent.data, sessionId);
          strcpy(pkt_sent.source, source);
          pthread_mutex_lock(&sessionList_mutex);
          // sessionList = join_session(sessionList, sessionId, newUsr);
          strcpy(newClient->clientID, source);
          newClient->joinedSession = NULL;
          newClient->nextCoClient = NULL;
          // newClient2->serverIP;
          // newClient2->serverPORT;
          JoinSess(newClient, sessionList, sessionId);
          pthread_mutex_unlock(&sessionList_mutex);
          printf("User %s: Succeeded join session %s\n", newClient->clientID,
                 sessionId);
          pthread_mutex_lock(&userLoggedin_mutex);
          pthread_mutex_unlock(&userLoggedin_mutex);
        }
      }
      else if (packet.type == LEAVE_SESS)
      {
        //printf("position 0\n");
        LeaveSess(newClient, sessionList);
        //printf("position 7\n");
      }

      else if (packet.type == NEW_SESS)
      {
        char sessId[MAX_DATA];
        strncpy(sessId, packet.data, MAX_DATA - 1);
        //printf("1:%s\n", sessId);
        //printf("0:%s\n", packet.data);
        CreateSess(sessId, sessionList);
        //printf("2:%s\n", sessionList->head->sessionID);
        JoinSess(newClient, sessionList, sessId);
        strcpy(pkt_sent.data, "successfully create a new session.");
        pkt_sent.type = NS_ACK;
        toSent = true;
      }
      else if (packet.type == MESSAGE)
      {
        pkt_sent.type = MESSAGE;
        strcpy(pkt_sent.source, newClient->clientID);
        strcpy(pkt_sent.data, packet.data);
        pkt_sent.size = strlen((char *)(pkt_sent.data));

        memset(buffer, 0, sizeof(char) * BUF_SIZE);
        packetToString(&pkt_sent, buffer);
        fprintf(stderr, "Server: Broadcasting message %s to session:", buffer);
        Session *cur_sess = sessionList->head;
        for (int i = 0; i < sessionList->list_sz; i++)
        {
          CoClient *cur_client = cur_sess->head;
          for (int j = 0; j < cur_sess->session_sz; j++)
          {
            if ((bytesSent =
                     send(cur_client->sockfd, buffer, BUF_SIZE - 1, 0)) == -1)
            {
              perror("error send\n");
              exit(1);
            }
            cur_client = cur_client->nextCoClient;
          }
          cur_sess = cur_sess->nextSession;
        }
        toSent = false;
      }
      else if (packet.type == QUERY)
      {
        // GetList();
        printf("preparing to print list to client\n");
        pkt_sent.type = QU_ACK;
        toSent = true;
        int index = 0;

        Session *cur_sess = sessionList->head;
        for (int i = 0; i < sessionList->list_sz; i++)
        {
          //printf("%s\n", cur_sess->sessionID);
          index += sprintf((char *)(pkt_sent.data) + index, "%s",
                           cur_sess->sessionID);
          CoClient *cur_client = cur_sess->head;
          for (int j = 0; j < cur_sess->session_sz; j++)
          {
            //printf("%s\n", cur_client->clientID);
            index += sprintf((char *)(pkt_sent.data) + index, "\t%s",
                             cur_client->clientID);
            cur_client = cur_client->nextCoClient;
          }
          pkt_sent.data[index++] = '\n';
          cur_sess = cur_sess->nextSession;
        }
      }
    }
    if (toSent)
    {
      //printf("preparing to send pack.\n");
      memcpy(pkt_sent.source, newClient->clientID, MAX_NAME);
      pkt_sent.size = strlen((char *)pkt_sent.data);

      memset(buffer, 0, BUF_SIZE);
      packetToString(&pkt_sent, buffer);
      printf("preparing to send pack:%s\n", buffer);
      if ((bytesRecvd = send(newClient->sockfd, buffer, BUF_SIZE - 1, 0)) == -1)
      {
        perror("error send\n");
      }
    }
    if (toExit)
      break;
  }
  close(newClient->sockfd);

  if (loggedIn == true)
  {
    /*CoClient* newClient2 = (newClient2, sessionList);
    memset(newClient2->sessionID, 0, (MAX_NAME) * sizeof(char));
    int index = FindClient(clientList, newClient2->clientID);
    Client* temp = clientList->head;
    for (int i = 0; i < index; i++) {
      temp = temp->next;
    }
    temp->isLog = false;
    loggedIn = false;
    pthread_mutex_lock(&userConnectedCnt_mutex);
    pthread_mutex_unlock(&userConnectedCnt_mutex);
  }
  return NULL;*/

    // leave all sessions before the client exits
    while (newClient->joinedSession != NULL)
    {
      LeaveSess(newClient, sessionList);
    }
    DestroyCoclient(newClient);
    int client_index = FindClient(clientList, newClient->clientID);
    Client *client_tmp = clientList->head;
    for (int i = 0; i < client_index; i++)
    {
      client_tmp = client_tmp->next;
    }
    client_tmp->isLog = false;
    free(newClient);
    loggedIn = false;

    pthread_mutex_lock(&userConnectedCnt_mutex);
    pthread_mutex_unlock(&userConnectedCnt_mutex);
  }
}

void print_err(char *str, int line, int err_no)
{
  printf("%d, %s :%s\n", line, str, strerror(err_no));
  _exit(-1);
}


/*
void *receive(void *pth_arg)
{
  int ret = 0;
  long cfd = (long)pth_arg;
  char buf[100] = {0};
  while (1)
  {
    
    bzero(&buf, sizeof(buf));
    ret = recv(cfd, &buf, sizeof(buf), 0);
    if (-1 == ret)
    {
      print_err("recv failed.\n", __LINE__, errno);
    }
    printf("server_thread tid is %lld \n", pthread_self()); 
    printf("recv from client: %s \n", buf);

    ret = send(cfd,&buf, sizeof(buf), 0);
    if (-1 == ret) print_err("send failed", __LINE__, errno);
    
  }
}
*/
int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("You need to provide the port number.\n");
  }

  clientList = createClientList();
  sessionList = createSessionList();

  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;

  FILE *file = fopen("UserData.txt", "r");
  if (file == NULL)
  {
    perror("Error opening file.\n");
    return 1;
  }

  char username[MAX_NAME];
  char password[MAX_DATA];


  while (fscanf(file, "%s %s", username, password) == 2)
  {
    //printf("1\n");
    insertClient(username, password, clientList);
  }
  fclose(file);

  int skfd = -1; // listen on sock_fd, new connection on new_fd
  int ret = -1;
  skfd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == skfd)
  {
    print_err("socket failed", __LINE__, errno);
  }

  struct sockaddr_in addr;
  char s[BUF_SIZE];
  addr.sin_family = AF_INET;           
  addr.sin_port = htons(atoi(argv[1]));
  addr.sin_addr.s_addr = INADDR_ANY;   

  ret = bind(skfd, (struct sockaddr *)&addr, sizeof(addr));
  if (-1 == ret)
  {
    print_err("bind failed", __LINE__, errno);
  }

  ret = listen(skfd, 3);
  if (-1 == ret)
  {
    print_err("listen failed", __LINE__, errno);
  }
  else
  {
    printf("Waiting for clients to log in...\n");
  }

 
  long cfd = -1; 
  pthread_t id;
 
  while (1)
  {
    CoClient *newClient = calloc(sizeof(CoClient), 1);
    sin_size = sizeof(their_addr);
    newClient->sockfd = accept(skfd, (struct sockaddr *)&their_addr, &sin_size);
    if (newClient->sockfd == -1)
    {
      perror("accept error");
      break;
    }
    //inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr),
    //         s, sizeof(s));

    //struct sockaddr_in caddr = {0};  // caddr:client_address
    //int csize = sizeof(caddr);
    //printf("Connection preparing.\n");

    //cfd = accept(skfd, (struct sockaddr*)&caddr, &csize);
    //printf("accepted.\n");
    //if (-1 == cfd) {
    //  print_err("accept failed", __LINE__, errno);
    //}
    /*else{
            cfds[n_client]=cfd;
            n_client++;
    }*/
 
    else
    {
      //printf("Connection with client established. cport = %d, caddr = %s\n",
      //ntohs(their_addr.sin_port), inet_ntoa(their_addr.sin_addr));
      printf("Connection set.\n");
    }

    //ret = send(cfd, "From server: Successfully log in.\n",
    //           sizeof("From server: Successfully log in.\n"), 0);
    //if (-1 == ret) print_err("send failed", __LINE__, errno);


    pthread_create(&id, NULL, ReceiveClient, (void *)newClient);
    //if (-1 == ret) print_err("accept failed", __LINE__, errno);
  }
  return 0;
}
