#include "ChatRoom.h"
#include <iostream>

json babble::loadConfig(std::string configfile)
{
  std::ifstream f(configfile);
  return json::parse(f);
}

// 返回接收到的报文的长度
int babble::recvPackage(int client_fd, struct BabblePackage &package)
{
  // recv()返回值为0表示对方关闭了连接，返回值为-1表示出错
  int num_recv = recvN(client_fd, (char *)&package.header, sizeof(package.header));
  if (num_recv < 0)
  {
    return -1;
  }
  if (num_recv == 0)
  {
    return 0;
  }
  char buff[package.header.length - sizeof(package.header)]; // 用于存放报文体
  num_recv = recvN(client_fd, buff, package.header.length - sizeof(package.header));
  if (num_recv < 0)
  {
    return -1;
  }
  if (num_recv == 0)
  {
    return 0;
  }
  package.body = json::parse(std::string(buff));
  std::cout << "@RecvN:" << package.header.length << package.body.dump() << std::endl;
  return package.header.length;
}

// 返回发送的报文体的长度(各个错误情况下，返回的值和send相同)
int babble::sendPackage(int client_fd, struct BabblePackage package)
{
  const std::string json_str = package.body.dump();
  int len = std::strlen(json_str.c_str());
  int total_size = sizeof(package.header) + len;
  package.header.length = total_size;
  char buff[total_size];
  memcpy(buff, &package.header, sizeof(package.header));
  memcpy(buff + sizeof(package.header), json_str.c_str(), len);
  return sendN(client_fd, buff, total_size);
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