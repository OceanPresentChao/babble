#include "chatroom.h"
#include <iostream>

std::string babble::formatMessage(struct BabbleMessage message)
{
  json j;
  j["type"] = message.type;
  j["code"] = message.code;
  j["message"] = message.message;
  j["to"] = message.to;
  j["from"] = message.from;
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
  memset(&pkg, 0, sizeof(pkg));
  // recv()返回值为0表示对方关闭了连接，返回值为-1表示出错
  int num_recv = recvN(client_fd, (char *)&pkg.length, BabblePkgWidth);
  if (num_recv < 0)
  {
    return -1;
  }
  if (num_recv == 0)
  {
    return 0;
  }
  num_recv = recvN(client_fd, (char *)&pkg.message, pkg.length);
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
int babble::sendMessage(int client_fd, struct BabbleMessage message)
{
  std::string wrappedMsg = formatMessage(message);
  BabblePackage pkg;
  memset(&pkg, 0, sizeof(pkg));
  pkg.length = strlen(wrappedMsg.c_str());
  memcpy(pkg.message, wrappedMsg.c_str(), pkg.length);
  sendN(client_fd, (char *)&pkg, pkg.length + BabblePkgWidth);
  return pkg.length;
}

int babble::recvN(int client_fd, char *buffer, int length)
{
  int num_recv = 0;
  int num_left = length;
  while (num_left > 0)
  {
    num_recv = recv(client_fd, buffer, num_left, 0);
    if (num_recv < 0)
    {
      return -1;
    }
    if (num_recv == 0)
    {
      return 0;
    }
    num_left -= num_recv;
    buffer += num_recv;
  }
  return length;
}

int babble::sendN(int client_fd, char *buffer, int length)
{
  int num_send = 0;
  int num_left = length;
  while (num_left > 0)
  {
    num_send = send(client_fd, buffer, num_left, 0);
    if (num_send < 0)
    {
      return -1;
    }
    if (num_send == 0)
    {
      return 0;
    }
    num_left -= num_send;
    buffer += num_send;
  }
  return length;
}