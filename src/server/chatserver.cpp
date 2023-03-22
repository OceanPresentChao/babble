#include "chatserver.h"
#include "../common/chatroom.h"
#include <iostream>

ChatServer::ChatServer() : max_connection(10)
{
  this->max_fd = 0;
  FD_ZERO(&this->fds);
  this->client_fds.resize(this->max_connection + 1, 0);
  this->client_addrs.resize(this->max_connection + 1);
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

void ChatServer::run()
{
  while (true)
  {
    FD_ZERO(&this->fds);
    // 标准输入
    FD_SET(STDIN_FILENO, &this->fds); // 用于在文件描述符集合中增加一个新的文件描述符。STDIN_FILENO是标准输入文件描述符
    this->max_fd = std::max(this->max_fd, STDIN_FILENO);
    // 服务器端socket
    FD_SET(this->sv_socket, &this->fds); // 把要检测的套接字server_sock_fd加入到集合中
    this->max_fd = std::max(this->max_fd, this->sv_socket);

    for (int i = 1; i <= this->max_connection; i++)
    {
      if (this->client_fds[i] != 0)
      {
        this->max_fd = std::max(this->max_fd, client_fds[i]);
        FD_SET(this->client_fds[i], &this->fds);
      }
    }
    int nfds = select(this->max_fd + 1, &this->fds, NULL, NULL, NULL);
    if (nfds < 0)
    {
      std::cout << "select error" << std::endl;
      continue;
    }
    else
    {
      if (FD_ISSET(this->sv_socket, &this->fds))
      {
        this->handleNewConnection();
      }

      for (int i = 1; i <= this->max_connection; i++)
      {
        if (client_fds[i] && FD_ISSET(this->client_fds[i], &this->fds))
        {
          this->handleRecvMessage(this->client_fds[i]);
        }
      }
    }

    if (this->ct_socket == -1)
    {
      perror("Accept failed");
      exit(1);
    }
  }

  this->stop();
}

int ChatServer::stop()
{
  close(this->sv_socket);
  printf("Close Server\n");
  return 0;
}

void ChatServer::handleNewConnection()
{
  std::cout << "handleNewConnection" << std::endl;
  struct sockaddr_in client_address;
  socklen_t address_len = sizeof(struct sockaddr_in);
  char buff[BUFFSIZE];
  this->ct_socket = accept(this->sv_socket, (struct sockaddr *)&client_address, &address_len);
  if (this->ct_socket > 0)
  {
    int index = -1;
    for (int i = 1; i <= this->max_connection; i++)
    {
      if (this->client_fds[i] == 0)
      {
        index = i;
        this->client_fds[i] = this->ct_socket;
        break;
      }
    }
    if (index >= 0)
    {
      printf("新客户端(%d)加入成功 %s:%d \n", index, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
      // inet_ntoa:接受一个in_addr结构体类型的参数并返回一个以点分十进制格式表示的IP地址字符串
      this->client_addrs[index] = client_address;
      // sendIP(index);
    }
    else
    {
      memset(buff, 0, sizeof(buff));
      strcpy(buff, "服务器加入的客户端数达到最大值!\n");
      send(this->ct_socket, buff, BUFFSIZE, 0);
      printf("客户端连接数达到最大值，新客户端加入失败 %s:%d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
    }
  }
}

void ChatServer::handleRecvMessage(int client_fd)
{
  char buff[BUFFSIZE];
  bzero(buff, BUFFSIZE);
  // 对应伪代码中的recv(connfd, buff);
  int num_recv = recv(client_fd, buff, BUFFSIZE, 0);
  // recv()返回值为0表示对方关闭了连接，返回值为-1表示出错
  if (num_recv < 0)
  {
    perror("Recv failed");
    FD_CLR(client_fd, &this->fds);
    return;
  }
  if (num_recv == 0)
  {
    // 若向已关闭的socket发送消息，内核会向进程发送一个SIGPIPE信号，进程默认行为是终止进程
    std::cout << "Client disconnected" << std::endl;
    FD_CLR(client_fd, &this->fds);
    for (int i = 1; i <= this->max_connection; i++)
    {
      if (this->client_fds[i] == client_fd)
      {
        this->client_fds[i] = 0;
        break;
      }
    }
    return;
  }
  // 正常接受到消息
  printf("Recv: %s\n", buff);
  memcpy(buff, "Hello, I have received your message.", BUFFSIZE);
  send(this->ct_socket, buff, BUFFSIZE, 0);
}