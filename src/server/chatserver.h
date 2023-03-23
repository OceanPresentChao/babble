#include <cstring>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/event.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <set>
#include <cmath>
#include <map>
#include "chatroom.h"

class ChatServer
{
public:
  int sv_socket;
  int port;
  struct sockaddr_in server_address;

private:
  int max_connection;
  int max_fd;
  fd_set fds;
  std::set<int> client_fds;
  std::map<int, struct sockaddr_in> client_addrs;

public:
  ChatServer();
  ~ChatServer();
  void init();
  void setPort(int port);
  int listenClient();
  void run();
  int stop();

private:
  void handleNewConnection();
  void handleNewMessage(int client_fd);
  void handleClientExit(int client_fd);
  void broadcastMessage(babble::BabbleProtocol code, std::string message, const std::set<int> &group);
  void sendMessage(int client_fd, babble::BabbleProtocol code, std::string message);
  int getOnlineCount();
  std::string getClientName(int client_fd);
};