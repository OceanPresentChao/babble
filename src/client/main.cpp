#include <signal.h>
#include <iostream>
#include "chatclient.h"
#include "../common/chatroom.h"
using json = nlohmann::json;

json Config;
ChatClient Client;

void stopClient(int p)
{
  Client.disconnect();
  std::cout << "client stop!" << std::endl;
  exit(0);
}

int main()
{
  signal(SIGINT, stopClient);
  Config = babble::loadConfig("./config.json");
  Client.setHost(Config["client"]["host"]);
  Client.setPort(Config["client"]["port"]);
  Client.init();
  int flag;
  flag = Client.connectServer();
  if (flag == -1)
  {
    std::cout << "Connection failed!";
    return 0;
  }
  else
  {
    std::cout << "Connection success!";
  }
  Client.run();
  return 0;
}
