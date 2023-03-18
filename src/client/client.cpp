#include "client.h"
#include "../common/chatroom.h"

Client::Client(std::string host, int port) : port(port), host(host)
{
}

Client::~Client()
{
  this->disconnect();
}

void Client::init()
{
  // bzero() 会将内存块（字符串）的前n个字节清零;
  bzero(&this->server_address, sizeof(this->server_address));
  this->server_address.sin_family = AF_INET;
  inet_pton(AF_INET, this->host.c_str(), &this->server_address.sin_addr);
  this->server_address.sin_port = htons(this->port);
}

int Client::connectServer()
{
  this->ct_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (this->ct_socket == -1)
  {
    printf("Create socket error(%d): %s\n", errno, strerror(errno));
    return -1;
  }
  if (connect(this->ct_socket, (struct sockaddr *)&this->server_address, sizeof(this->server_address)) == -1)
  {
    printf("Connect error(%d): %s\n", errno, strerror(errno));
    return -1;
  }
  return 0;
}

int Client::disconnect()
{
  close(this->ct_socket);
  return 0;
}

int Client::sendMessage()
{
  char buff[BUFFSIZE];
  bzero(buff, sizeof(buff));
  printf("Please input: ");
  scanf("%s", buff);
  send(this->ct_socket, buff, strlen(buff), 0);
  return 0;
}