#include "chatserver.h"
#include "../common/chatroom.h"

ChatServer::ChatServer()
{
}

ChatServer::~ChatServer()
{
  this->stop();
}

void ChatServer::setPort(int port)
{
  this->port = port;
}

void ChatServer::init()
{
  // bzero() 会将内存块（字符串）的前n个字节清零;
  bzero(&this->server_address, sizeof(this->server_address));
  this->server_address.sin_family = AF_INET;
  this->server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  this->server_address.sin_port = htons(this->port);
}

int ChatServer::listenClient()
{
  this->sv_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (this->sv_socket == -1)
  {
    perror("Socket creation failed");
    return -1;
  }
  if (bind(this->sv_socket, (struct sockaddr *)&this->server_address, sizeof(this->server_address)) == -1)
  {
    perror("Bind failed");
    return -1;
  }

  if (listen(this->sv_socket, 2048) == -1)
  {
    perror("Listen failed");
    return -1;
  }
  return 0;
}

void ChatServer::start()
{
  char buff[BUFFSIZE];
  while (true)
  {
    this->ct_socket = accept(this->sv_socket, NULL, NULL);
    if (this->ct_socket == -1)
    {
      perror("Accept failed");
      exit(1);
    }
    bzero(buff, BUFFSIZE);
    // 对应伪代码中的recv(connfd, buff);
    recv(this->ct_socket, buff, BUFFSIZE - 1, 0);
    printf("Recv: %s\n", buff);
    close(this->ct_socket);
  }

  this->stop();
}

int ChatServer::stop()
{
  close(this->sv_socket);
  printf("Close Server\n");
  return 0;
}