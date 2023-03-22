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

class ChatClient
{
public:
  int ct_socket;
  struct sockaddr_in server_address;

public:
  ChatClient();
  ~ChatClient();
  void setHost(std::string host);
  void setPort(int port);
  void init();
  int connectServer();
  int sendMessage();
  int disconnect();

private:
  int port;
  std::string host;
};