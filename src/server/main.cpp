#include <iostream>
#include "server.h"
#include "../common/chatroom.h"

Server server(DEFAULT_PORT);

void stopServer(int p)
{
  server.stop();
  std::cout << "server stop!" << std::endl;
  exit(0);
}

int main()
{
  signal(SIGINT, stopServer);
  int flag;
  server.init();
  flag = server.listenClient();
  if (flag != 0)
  {
    perror("listen failed!");
    return 0;
  }
  else
  {
    std::cout << "listen success!" << std::endl;
  }
  server.start();
}
