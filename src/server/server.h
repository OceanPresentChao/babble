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

class Server
{
public:
  int ct_socket;
  int sv_socket;
  int port;
  struct sockaddr_in server_address;

public:
  Server(int port);
  ~Server();
  void init();
  int listenClient();
  void start();
  int stop();
};