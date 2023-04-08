#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H
#include <cstdio>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <thread>
#include "chatroom.h"

enum ClientStatus
{
  IDLE = 0,
  CHATTING = 1,
};

class ChatClient
{
public:
  int ct_socket;
  struct sockaddr_in server_address;

private:
  int port;
  bool isRunning;
  ClientStatus status;
  std::string host;
  std::thread recv_thread;
  json receiveMessage();
  int sendMessage(babble::BabbleMessage);

public:
  ChatClient();
  ~ChatClient();
  void setHost(std::string host);
  void setPort(int port);
  void init();
  void run();
  int connectServer();
  int disconnect();
  void handleGroupChat();
  void handlePrivateChat();
  static void handleReceiveChat(void *client);

  // static void receiveMessage(void *client);
};
#endif