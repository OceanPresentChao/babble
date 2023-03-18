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
#include <mutex>
#include <vector>

class Client
{
public:
  int ct_socket;
  struct sockaddr_in server_address;

public:
  Client(std::string host, int port);
  ~Client();
  void init();
  int connectServer();
  int sendMessage();
  int disconnect();

private:
  int port;
  std::string host;
};