#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/event.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <vector>
#include <cmath>
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
  std::vector<int> client_fds;
  std::vector<struct sockaddr_in> client_addrs;

public:
  ChatServer();
  ~ChatServer();
  void init();
  void setPort(int port);
  int listenClient();
  void handleNewConnection();
  void handleNewMessage(int client_fd);
  void sendMessage(int client_fd, babble::BabbleProtocol type, int code, std::string message);
  // void broadCast(babble::BabbleProtocol type, int code, std::string message);
  void run();
  int stop();
};