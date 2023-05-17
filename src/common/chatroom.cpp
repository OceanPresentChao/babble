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
  ssize_t num_recv = recvN(client_fd, (char *)&package.header, BabblePkgHdrWidth);
  if (num_recv < 0)
  {
    return -1;
  }
  if (num_recv == 0)
  {
    return 0;
  }
  char buff[package.header.length - BabblePkgHdrWidth + 1]; // 用于存放报文体
  num_recv = recvN(client_fd, buff, package.header.length - BabblePkgHdrWidth);
  if (num_recv < 0)
  {
    return -1;
  }
  if (num_recv == 0)
  {
    return 0;
  }
  buff[package.header.length - BabblePkgHdrWidth] = '\0'; // json::parse()要求字符串以'\0'结尾
  try
  {
    package.body = json::parse(std::string(buff));
  }
  catch (std::exception e)
  {
    std::cout << "json parse error" << std::endl;
    return -1;
  }
  std::cout << "@RecvN:" << package.header.length << package.body.dump() << std::endl;
  return package.header.length;
}

// 返回发送的报文体的长度(各个错误情况下，返回的值和send相同)
int babble::sendPackage(int client_fd, struct BabblePackage package)
{
  const std::string json_str = package.body.dump();
  ssize_t len = json_str.length();
  ssize_t total_size = BabblePkgHdrWidth + len;
  package.header.length = total_size;
  char buff[total_size];
  memcpy(buff, &package.header, BabblePkgHdrWidth);
  memcpy(buff + BabblePkgHdrWidth, json_str.c_str(), len);
  return sendN(client_fd, buff, total_size);
}

int babble::recvN(int client_fd, char *buffer, int length)
{
  ssize_t num_recv = 0;
  ssize_t num_left = length;
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
  ssize_t num_send = 0;
  ssize_t num_left = length;
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