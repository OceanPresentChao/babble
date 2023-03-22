#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <thread>
#include <mutex>
#include <vector>

class ChatServer
{
public:
  int ct_socket;
  int sv_socket;
  int port;
  struct sockaddr_in server_address;

public:
  ChatServer();
  ~ChatServer();
  void init();
  void setPort(int port);
  int listenClient();
  void start();
  int stop();
};