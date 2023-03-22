#include <iostream>
#include "chatclient.h"
#include "../common/chatroom.h"

json Config;
ChatClient client;

int main()
{
  Config = loadConfig("./config.json");
  client.setHost(Config["client"]["host"]);
  client.setPort(Config["client"]["port"]);
  client.init();
  int flag;
  flag = client.connectServer();
  if (flag == -1)
  {
    std::cout << "Connection failed!";
    return 0;
  }
  else
  {
    std::cout << "Connection success!";
  }
  client.sendMessage();
  return 0;
}
