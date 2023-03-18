#include "client.h"
#include "../common/chatroom.h"
#include <iostream>
int main()
{
  int flag;
  Client client("127.0.0.1", DEFAULT_PORT);
  client.init();
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
