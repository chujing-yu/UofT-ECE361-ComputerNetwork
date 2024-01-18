#ifndef global_h
#define global_h
#define MAX_NAME 32
#define MAX_DATA 512
#define BUF_SIZE 600
enum TYPE
{
  LOGIN,
  LO_ACK,
  LO_NAK,
  EXIT, //3
  JOIN,
  JN_ACK, //5
  JN_NAK,
  LEAVE_SESS, //7
  NEW_SESS,   //8
  NS_ACK,     //9
  MESSAGE,    //10
  QUERY,
  QU_ACK //12
};

#endif