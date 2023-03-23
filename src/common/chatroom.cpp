#include "chatroom.h"
#include <iostream>

std::string babble::formatMessage(enum BabbleProtocol type, int code, std::string message)
{
  json j;
  j["type"] = type;
  j["code"] = code;
  j["message"] = message;
  return j.dump();
}

json babble::parseMessage(std::string message)
{
  return json::parse(message);
}

json babble::loadConfig(std::string configfile)
{
  std::ifstream f(configfile);
  return json::parse(f);
}

// 返回接收到的报文体的长度
int babble::recvMessage(int client_fd, std::string &message)
{
  BabblePackage pkg;
  // recv()返回值为0表示对方关闭了连接，返回值为-1表示出错
  int num_recv = recv(client_fd, &pkg.length, BabblePkgWidth, 0);
  if (num_recv < 0)
  {
    return -1;
  }
  if (num_recv == 0)
  {
    return 0;
  }
  num_recv = recv(client_fd, &pkg.message, pkg.length, 0);
  if (num_recv < 0)
  {
    return -1;
  }
  if (num_recv == 0)
  {
    return 0;
  }
  message = pkg.message;
  return pkg.length;
}

// 返回发送的报文体的长度
int babble::sendMessage(int client_fd, babble::BabbleProtocol type, int code, std::string message)
{
  std::string wrappedMsg = formatMessage(type, code, message);
  int msglen = wrappedMsg.size();
  BabblePackage pkg;
  pkg.length = msglen;
  memcpy(pkg.message, wrappedMsg.c_str(), msglen);
  send(client_fd, (void *)&pkg, msglen + BabblePkgWidth, 0);
  return msglen;
}