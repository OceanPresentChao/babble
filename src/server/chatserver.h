#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/event.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <algorithm>
#include <set>
#include <cmath>
#include <map>
#include <vector>
#include "chatroom.h"

class ChatServer
{
public:
  int sv_socket;
  int port;
  struct sockaddr_in server_address;

private:
  int max_connection;
  std::map<int, struct sockaddr_in> client_addrs;
  std::set<int> client_fds;
  std::map<int, std::vector<int>> client_groups;
  // io multiplexing
  int max_fd;
  fd_set fds;

public:
  ChatServer();
  ~ChatServer();
  void init();
  void setPort(int port);
  int listenClient();
  void run();
  int stop();
  void sendPrivateMessage(int client_fd, babble::BabbleProtocol code, std::string message);
  void sendBroadcastMessage(babble::BabbleProtocol code, std::string message, const std::set<int> &group);

private:
  void handleNewConnection();
  void handleNewMessage(int client_fd);
  void handleClientExit(int client_fd);
  int getOnlineCount();
  int getGroupOnlineCount(int group_id);
  json getOnlineList();
  std::string getClientName(int client_fd);
};
#endif