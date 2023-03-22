#include "chatserver.h"
#include "../common/chatroom.h"
#include <iostream>

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
  struct kevent events_to_monitor[1];
  struct kevent event_data[1];

  return 0;
}

void ChatServer::run()
{
  char buff[BUFFSIZE];
  this->ct_socket = accept(this->sv_socket, NULL, NULL);
  while (true)
  {
    if (this->ct_socket == -1)
    {
      perror("Accept failed");
      exit(1);
    }
    bzero(buff, BUFFSIZE);
    // 对应伪代码中的recv(connfd, buff);
    int num_recv = recv(this->ct_socket, buff, BUFFSIZE - 1, 0);
    if (num_recv < 0)
    {
      perror("Recv failed");
      break;
    }
    if (num_recv == 0)
    {
      // 若向已关闭的socket发送消息，内核会向进程发送一个SIGPIPE信号，进程默认行为是终止进程
      std::cout << "Client disconnected" << std::endl;
      break;
    }
    printf("Recv: %s\n", buff);
    memcpy(buff, "Hello, I have received your message.", BUFFSIZE);
    send(this->ct_socket, buff, BUFFSIZE, 0);
  }

  this->stop();
}

int ChatServer::stop()
{
  close(this->sv_socket);
  printf("Close Server\n");
  return 0;
}