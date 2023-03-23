#include "chatserver.h"
#include "../common/chatroom.h"
#include <iostream>

ChatServer::ChatServer() : max_connection(10)
{
  this->max_fd = 0;
  FD_ZERO(&this->fds);
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

    for (int fd : this->client_fds)
    {
      if (fd != 0)
      {
        this->max_fd = std::max(this->max_fd, fd);
        FD_SET(fd, &this->fds);
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

      for (int fd : this->client_fds)
      {
        if (fd && FD_ISSET(fd, &this->fds))
        {
          this->handleNewMessage(fd);
        }
      }
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
  struct sockaddr_in client_address;
  socklen_t address_len = sizeof(struct sockaddr_in);
  char buff[BUFFSIZE];
  int ct_socket = accept(this->sv_socket, (struct sockaddr *)&client_address, &address_len);
  if (ct_socket == -1)
  {
    perror("Accept failed");
    exit(1);
  }
  if (ct_socket > 0)
  {
    if (this->client_fds.size() < this->max_connection)
    {
      // inet_ntoa:接受一个in_addr结构体类型的参数并返回一个以点分十进制格式表示的IP地址字符串
      this->client_fds.insert(ct_socket);
      this->client_addrs[ct_socket] = client_address;

      std::set<int> group(this->client_fds.begin(), this->client_fds.end());
      group.erase(ct_socket);
      std::string message = this->getClientName(ct_socket) + "加入聊天室";
      this->broadcastMessage(babble::BabbleProtocol::JOIN, 200, message, group);

      message = "欢迎加入聊天室，当前在线人数：" + std::to_string(this->getOnlineCount()) + "\n";
      this->sendMessage(ct_socket, babble::BabbleProtocol::MESSAGE, 200, message);
    }
    else
    {
      this->sendMessage(ct_socket, babble::BabbleProtocol::INVALID, 300, "服务器加入的客户端数达到最大值!\n");
      printf("客户端连接数达到最大值，新客户端加入失败 %s:%d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
    }
  }
}

void ChatServer::handleNewMessage(int client_fd)
{
  std::string message = "";
  int num_recv = babble::recvMessage(client_fd, message);
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
    this->client_fds.erase(client_fd);
    this->client_addrs.erase(client_fd);
    return;
  }
  // 正常接受到消息
  std::cout << "Recv:" << message << std::endl;
  std::string bmsg = this->getClientName(client_fd) + "说：\n" + message;
  std::set<int> group(this->client_fds.begin(), this->client_fds.end());
  this->broadcastMessage(babble::BabbleProtocol::MESSAGE, 200, bmsg, group);
}

inline void ChatServer::sendMessage(int client_fd, babble::BabbleProtocol type, int code, std::string message)
{
  babble::sendMessage(client_fd, type, code, message);
  return;
}

void ChatServer::broadcastMessage(babble::BabbleProtocol type, int code, std::string message, const std::set<int> &group)
{
  for (int fd : group)
  {
    this->sendMessage(fd, type, code, message);
  }
}

int ChatServer::getOnlineCount()
{
  return this->client_fds.size() - std::count(this->client_fds.begin(), this->client_fds.end(), 0);
}

std::string ChatServer::getClientName(int client_fd)
{
  struct sockaddr_in &client_address = this->client_addrs[client_fd];
  std::string name = "客户端" + std::string(inet_ntoa(client_address.sin_addr)) + std::to_string(ntohs(client_address.sin_port));
  return name;
}