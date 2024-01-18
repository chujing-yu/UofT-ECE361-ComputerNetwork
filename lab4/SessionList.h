#ifndef SessionList_h
#define SessionList_h
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "global.h"
/*void CreateSess(unsigned char* sessID, SessionList* sList);
int FindSession(SessionList* sList, unsigned char* sessID);
void JoinSess(CoClient* client, SessionList* sList, unsigned char* sessID);
void DestroySession(SessionList* sList, Session* sess);
CoClient* LeaveSess(CoClient* client, SessionList* sList);
void GetList(SessionList* sList);
bool CheckClient(CoClient* client, unsigned char* sessionID);*/

typedef struct _Session Session;

typedef struct _CoClient
{
  unsigned char clientID[MAX_NAME];
  unsigned char passwords[MAX_DATA];
  Session *joinedSession; // one client, one session
  unsigned char serverIP[MAX_NAME];
  unsigned char serverPORT[MAX_NAME];
  int sockfd;
  struct _CoClient *nextCoClient;
} CoClient;

typedef struct _Session
{
  CoClient *head;                    // point to the first CoClient
  int session_sz;                    // the number of nodes in a session
  unsigned char sessionID[MAX_NAME]; // session ID
  struct _Session *nextSession;      // point to the next session
} Session;

typedef struct _SessionList
{
  Session *head; // point to the first session
  int list_sz;   // the number of sessions in the list
} SessionList;

SessionList *createSessionList()
{
  //printf("saaaaaaaaaa\n");
  SessionList *newList = (SessionList *)malloc(sizeof(SessionList));
  if (newList == NULL)
  {
    perror("Error allocationg memory for ClientList.\n");
    exit(EXIT_FAILURE);
  }

  newList->head = NULL;
  newList->list_sz = 0;

  return newList;
}
// create a new session and add it to the end of the session list
void CreateSess(unsigned char *sessID, SessionList *sList)
{
  // initialize the new session
  Session *temp = (Session *)malloc(sizeof(Session));
  temp->head = NULL;
  temp->session_sz = 0;
  memcpy(temp->sessionID, sessID, MAX_NAME * sizeof(unsigned char));
  temp->nextSession = NULL;

  // put the new session at the end of the session list
  // if the session list is empty
  if (sList->head == NULL)
  {
    sList->head = temp;
  }
  // if the session list is not empty
  else
  {
    Session *end = sList->head;
    while (end->nextSession != NULL)
    {
      end = end->nextSession;
    }
    end->nextSession = temp;
  }
  sList->list_sz++;
}

// find a session
int FindSession(SessionList *sList, unsigned char *sessID)
{
  Session *temp = sList->head;
  for (int i = 0; i < sList->list_sz; i++)
  {
    if (strcmp((char *)temp->sessionID, (char *)sessID) == 0)
    {
      return i;
    }
    temp = temp->nextSession;
  }
  return -1; // not find the session in the list
}

// let a new client join the session
void JoinSess(CoClient *client, SessionList *sList, unsigned char *sessID)
{
  int index = FindSession(sList, sessID);
  // if the session ID is wrong
  if (index == -1)
  {
    printf("Error: Cannot find the session.\n");
    return;
  }
  // if the session ID exists
  else
  {
    Session *temp = sList->head;
    for (int i = 0; i < index; i++)
    {
      temp = temp->nextSession;
    }
    Session *newEnd = (Session *)malloc(sizeof(Session));
    newEnd->head = temp->head;
    newEnd->session_sz = temp->session_sz;
    newEnd->nextSession = NULL;
    strcpy(newEnd->sessionID, temp->sessionID);
    if (client->joinedSession == NULL)
    {
      client->joinedSession = newEnd;
    }
    else
    {

      Session *end = client->joinedSession;
      while (end->nextSession != NULL)
      {
        end = end->nextSession;
      }
      end->nextSession = newEnd;
    }
    // if the session is not empty
    if (temp->head != NULL)
    {
      CoClient *endClient = temp->head;
      while (endClient->nextCoClient != NULL)
      {
        endClient = endClient->nextCoClient;
      }
      endClient->nextCoClient = client;
      temp->session_sz++;
      return;
    }
    // if the session is empty
    else
    {
      temp->head = client;
      temp->session_sz++;
      return;
    }
  }
}

// delete the coclient
void DestroyCoclient(CoClient *client)
{
  if (client->joinedSession != NULL)
  {
    Session *temp1 = client->joinedSession;
    while (temp1 != NULL)
    {
      Session *temp2 = temp1->nextSession;
      free(temp1);
      temp1 = temp2;
    }
  }
}

// delete the session
void DestroySession(SessionList *sList, Session *sess)
{
  int sessIndex = FindSession(sList, sess->sessionID);
  // check if the session exists
  if (sessIndex == -1)
  {
    printf("Error: Cannot find the session in the session list.\n");
    return;
  }
  CoClient *temp1 = sess->head;
  for (int i = 0; i < sess->session_sz; i++)
  {
    CoClient *temp2 = temp1->nextCoClient;
    DestroyCoclient(temp1);
    free(temp1);
    temp1 = temp2;
  }
  sess->head = NULL;
  // if the session is the head of session list
  if (sessIndex == 0)
  {
    sList->head = sess->nextSession;
    sess->nextSession = NULL;
    sList->list_sz--;
    free(sess);
  }
  // not the head of the session list
  else
  {
    Session *pre = sList->head;
    for (int i = 0; i < sessIndex - 1; i++)
    {
      pre = pre->nextSession;
    }
    pre->nextSession = sess->nextSession;
    sess->nextSession = NULL;
    sList->list_sz--;
    free(sess);
  }
}

// let the client leave the session
CoClient *LeaveSess(CoClient *client, SessionList *sList)
{
  Session *end = client->joinedSession;
  //printf("position 1\n");
  while (end->nextSession != NULL)
  {
    end = end->nextSession;
  }
  //printf("position 2\n");
  //printf("%s\n", end->sessionID);
  unsigned char sessID[MAX_NAME];
  strcpy(sessID, end->sessionID);

  //printf("%s\n", sessID);
  //printf("position 3\n");
  if (end == client->joinedSession)
  {
    //printf("position 4\n");
    free(client->joinedSession);
    client->joinedSession = NULL;
  }
  else
  {
    //printf("position 5\n");
    Session *pre_end = client->joinedSession;
    while (pre_end->nextSession != end)
    {
      pre_end = pre_end->nextSession;
    }
    pre_end->nextSession = end->nextSession;
    end->nextSession = NULL;
    free(end);
  }
  //printf("position 6\n");
  // find the session

  int sessIndex = FindSession(sList, sessID);
  if (sessIndex == -1)
  {
    printf("Error: Cannot find the session.\n");
    return NULL;
  }
  else
  {
    Session *sess = sList->head;
    //printf("sess idx: %d\n", sessIndex);
    for (int i = 0; i < sessIndex; i++)
    {
      sess = sess->nextSession;
    }
    //printf("position 6-1\n");
    CoClient *pre = sess->head;
    //printf("position 6-2\n");
    // remove the first client of a session
    if (pre == client)
    {
      //printf("position 6-3\n");
      // if it is the only one client in the session, remove the client and
      // delete the session
      if (pre->nextCoClient == NULL)
      {
        //printf("position 6-4\n");
        //sess->head = NULL;
        // delete the sessionsessionID
        DestroySession(sList, sess);
        //printf("position 6-5\n");
        return pre;
      }
      // if there are other clients in the session, just remove the chosen
      // client
      else
      {
        sess->head = pre->nextCoClient;
        pre->nextCoClient = NULL;
        sess->session_sz--;
        return pre;
      }
    }
    // remove the following client of the session
    else
    {
      // find the client prior to the chosen client
      while (pre != NULL)
      {
        if (pre->nextCoClient == client)
          break;
        else
          pre = pre->nextCoClient;
      }
      // if not find
      if (pre == NULL)
      {
        printf("Error: Cannot find the client in the session.\n");
        return NULL;
      }
      // find the client
      else
      {
        CoClient *cur = pre->nextCoClient;
        pre->nextCoClient = cur->nextCoClient;
        cur->nextCoClient = NULL;
        sess->session_sz--;
        return cur;
      }
    }
  }
}

// get the list of the connected clients and available sessions
void GetList(SessionList *sList)
{
  if (sList->head == NULL)
  {
    printf("No session.\n");
    return;
  }
  for (int i = 0; i < sList->list_sz; i++)
  {
    Session *sess = sList->head + i;
    printf("Session %s:\n", sess->sessionID);
    for (int j = 0; j < sess->session_sz; j++)
    {
      CoClient *client = sess->head + j;
      printf("%s\n", client->clientID);
    }
  }
}

// check if the coclient is in the session: true: is in the session  false: newEndnot
// in the session
bool CheckClient(CoClient *client, unsigned char *sessionID)
{
  Session *temp = client->joinedSession;
  if (temp == NULL)
  {
    return false;
  }
  else
  {
    while (temp != NULL)
    {
      if (strcmp((char *)temp->sessionID, (char *)sessionID) == 0)
      {
        return true;
      }
      temp = temp->nextSession;
    }
    return false;
  }
}
#endif