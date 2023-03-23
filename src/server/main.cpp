#include <iostream>
#include <fstream>
#include <string>
#include "chatserver.h"
#include "../common/chatroom.h"
using json = nlohmann::json;

json Config;
ChatServer Server;

void stopServer(int p)
{
  Server.stop();
  std::cout << "server stop!" << std::endl;
  exit(0);
}

int main()
{
  signal(SIGINT, stopServer);
  Config = babble::loadConfig("config.json");
  Server.setPort(Config["server"]["port"]);
  Server.init();
  int flag;
  flag = Server.listenClient();
  if (flag != 0)
  {
    perror("listen failed!");
    return 0;
  }
  else
  {
    std::cout << "listen success!" << std::endl;
  }
  Server.run();
}
