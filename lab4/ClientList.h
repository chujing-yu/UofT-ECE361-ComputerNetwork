#ifndef ClientList_h
#define ClientList_h
#include <stdio.h>
#include <string.h>

#include "global.h"
/*int FindClient(ClientList*, unsigned char*);
bool CheckPassword(ClientList*, int, unsigned char*);
void insertClient(char* userName, char* password)*/

typedef struct _Client
{
  unsigned char clientID[MAX_NAME];
  unsigned char passwords[MAX_DATA];
  bool isLog; // 0: not log in  1: have logged in
  struct _Client *next;
} Client;

typedef struct _ClientList
{
  Client *head; // head node of the client list
  int size;     // number of nodes in the list
} ClientList;

ClientList *createClientList()
{
  //printf("sdfasdfssss\n");
  ClientList *newList = (ClientList *)malloc(sizeof(ClientList));
  if (newList == NULL)
  {
    perror("Error allocationg memory for ClientList.\n");
    exit(EXIT_FAILURE);
  }

  newList->head = NULL;
  newList->size = 0;

  return newList;
}
int FindClient(ClientList *cList, unsigned char *id)
{
  Client *temp = cList->head;
  for (int i = 0; i < cList->size; i++)
  {
    if (strcmp(temp->clientID, id) == 0)
    {
      return i;
    }
    temp = temp->next;
  }
  return -1; // not find the client id in the list
}


void insertClient(char *userName, char *password, ClientList *cList)
{

  Client *new_client = (Client *)malloc(sizeof(Client));
  if (new_client == NULL)
  {
    perror("Error allocating memory");
    exit(1);
  }


  strncpy(new_client->clientID, userName, MAX_NAME - 1);
  strncpy(new_client->passwords, password, MAX_DATA - 1);
  new_client->isLog = false;

  new_client->next = cList->head;


  cList->head = new_client;
  cList->size++;
}

bool CheckPassword(ClientList *cList, int index, unsigned char *pwd)
{
  Client *temp = cList->head;
  for (int i = 0; i < index; i++)
  {
    temp = temp->next;
  }

  if (strcmp(temp->passwords, pwd) == 0)
  {
    return true;
  }
  return false;
}
#endif