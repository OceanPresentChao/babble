#include <cstdio>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <thread>
#include "chatroom.h"

class ChatClient
{
public:
  int ct_socket;
  struct sockaddr_in server_address;

private:
  int port;
  std::string host;

public:
  ChatClient();
  ~ChatClient();
  void setHost(std::string host);
  void setPort(int port);
  void init();
  void run();
  int connectServer();
  int disconnect();
  int sendMessage(std::string);
  static void receiveMessage(int client_socket);
};