#include <cstring>
#include <cstdlib>
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

class ChatServer
{
public:
  int ct_socket;
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
  void handleRecvMessage(int client_fd);
  void run();
  int stop();
};