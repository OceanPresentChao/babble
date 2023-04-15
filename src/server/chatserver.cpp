#include "ChatServer.h"
#include "../common/ChatRoom.h"
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
  // 获取文件标识符
  int flags = fcntl(this->sv_socket, F_GETFL, 0);
  if (flags < 0)
  {
    perror("fcntl failed");
    return -1;
  }
  // 设置非阻塞标志：在获取标志后，将 O_NONBLOCK 标志设置到 flags 中
  flags |= O_NONBLOCK;
  // 更新文件描述符的标志：使用 fcntl 函数更新文件描述符的标志。
  int ret = fcntl(this->sv_socket, F_SETFL, flags);
  if (ret < 0)
  {
    perror("fcntl failed");
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

    for (auto it = this->client_fds.begin(); it != this->client_fds.end(); it++)
    {
      int fd = *it;
      if (fd != 0)
      {
        this->max_fd = std::max(this->max_fd, fd);
        FD_SET(fd, &this->fds);
      }
    }
    int nfds = select(this->max_fd + 1, &this->fds, NULL, NULL, NULL);
    if (nfds <= 0)
    {
      continue;
    }
    else
    {
      if (FD_ISSET(this->sv_socket, &this->fds))
      {
        this->handleNewConnection();
      }
      else
      {
        // 单独copy一份set是因为在handleNewMessage中可能会进入handleClientExit修改client_fds
        std::set<int> tmp(this->client_fds.begin(), this->client_fds.end());
        for (int fd : tmp)
        {
          if (this->client_fds.count(fd) && fd && FD_ISSET(fd, &this->fds))
          {
            this->handleNewMessage(fd);
          }
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
    return;
  }
  if (ct_socket > 0)
  {
    if (this->client_fds.count(ct_socket))
    {
      return;
    }
    if (this->client_fds.size() < this->max_connection)
    {
      // inet_ntoa:接受一个in_addr结构体类型的参数并返回一个以点分十进制格式表示的IP地址字符串
      this->client_fds.insert(ct_socket);
      this->client_addrs[ct_socket] = client_address;
      FD_SET(ct_socket, &this->fds);
      this->max_fd = std::max(this->max_fd, ct_socket);

      std::cout << this->getClientName(ct_socket) << "加入服务器" << std::endl;
    }
    else
    {
      this->sendPrivateMessage(ct_socket, babble::BabbleProtocol::INVALID, "服务器加入的客户端数达到最大值!\n");

      printf("客户端连接数达到最大值，新客户端加入失败 %s:%d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
    }
  }
}

void ChatServer::handleNewMessage(int client_fd)
{
  std::string rawMsg = "";
  int num_recv = babble::recvMessage(client_fd, rawMsg);
  if (num_recv <= 0)
  {
    // 若向已关闭的socket发送消息，内核会向进程发送一个SIGPIPE信号，进程默认行为是终止进程
    this->handleClientExit(client_fd);
    return;
  }
  std::cout << "Recv:" << rawMsg << std::endl;

  json pkg = babble::parseMessage(rawMsg);
  babble::BabbleProtocol code = (babble::BabbleProtocol)pkg["code"].get<int>();
  babble::BabbleType type = (babble::BabbleType)pkg["type"].get<int>();
  std::string msg = pkg["message"].get<std::string>();
  int to = pkg["to"].get<int>();
  int from = pkg["from"].get<int>();

  // 收到客户端加入群聊消息
  if (code == babble::BabbleProtocol::JOIN)
  {
    int group_id = std::stoi(msg);
    this->client_groups[group_id].push_back(client_fd);

    std::set<int> group(this->client_groups[group_id].begin(), this->client_groups[group_id].end());
    group.erase(client_fd);
    std::string message = this->getClientName(client_fd) + "加入聊天室";
    this->sendBroadcastMessage(babble::BabbleProtocol::MESSAGE, message, group);

    message = "欢迎加入聊天室，当前在线人数：" + std::to_string(this->getGroupOnlineCount(group_id)) + "\n" + "您的名称为 " + this->getClientName(client_fd) + "\n";
    this->sendPrivateMessage(client_fd, babble::BabbleProtocol::MESSAGE, message);
  }
  // 收到客户端断开消息
  else if (code == babble::BabbleProtocol::LOGOUT)
  {
    this->handleClientExit(client_fd);
  }
  // 收到客户端离开群聊
  else if (code == babble::BabbleProtocol::EXIT)
  {
    int group_id = to;
    this->client_groups[group_id].erase(std::remove(this->client_groups[group_id].begin(), this->client_groups[group_id].end(), client_fd), this->client_groups[group_id].end());

    std::set<int> group(this->client_groups[group_id].begin(), this->client_groups[group_id].end());
    std::string message = this->getClientName(client_fd) + "离开聊天室";
    this->sendBroadcastMessage(babble::BabbleProtocol::MESSAGE, message, group);
    this->sendPrivateMessage(client_fd, babble::BabbleProtocol::OK, "会话已关闭");
  }
  // 查询当前在线人数
  else if (code == babble::BabbleProtocol::QUERY)
  {
    std::string message = "您的名称为 " + this->getClientName(client_fd) + "\n" + "当前在线人数：" + std::to_string(this->getOnlineCount()) + "\n";
    for (int fd : this->client_fds)
    {
      if (fd != 0)
      {
        message += this->getClientName(fd) + "\n";
      }
    }
    this->sendPrivateMessage(client_fd, babble::BabbleProtocol::MESSAGE, message);
  }
  // 接受客户端建立私聊会话请求
  else if (code == babble::BabbleProtocol::NEW_SESS)
  {
    if (!this->client_fds.count(to))
    {
      std::string message = "用户不存在";
      this->sendPrivateMessage(client_fd, babble::BabbleProtocol::INVALID, message);
    }
    else
    {
      std::string message = "建立会话成功";
      this->sendPrivateMessage(client_fd, babble::BabbleProtocol::NEW_SESS, message);
    }
  }
  else if (code == babble::BabbleProtocol::CLOSE_SESS)
  {
    this->sendPrivateMessage(client_fd, babble::BabbleProtocol::OK, "会话已关闭");
  }
  // 收到客户端群发消息
  else if (code == babble::BabbleProtocol::MESSAGE && type == babble::BabbleType::BROAD)
  {
    std::string message = this->getClientName(client_fd) + "说：\n" + msg;
    std::set<int> group(this->client_fds.begin(), this->client_fds.end());
    this->sendBroadcastMessage(babble::BabbleProtocol::MESSAGE, message, group);
  }
  // 收到客户端私发消息
  else if (code == babble::BabbleProtocol::MESSAGE && type == babble::BabbleType::PRIVATE)
  {
    std::string message = "(私聊)" + this->getClientName(client_fd) + "说：\n" + msg;
    if (this->client_fds.count(to))
    {
      this->sendPrivateMessage(to, babble::BabbleProtocol::MESSAGE, message);
    }
    else
    {
      this->sendPrivateMessage(client_fd, babble::BabbleProtocol::INVALID, "用户不存在\n");
    }
  }
}

void ChatServer::handleClientExit(int client_fd)
{

  std::string message = this->getClientName(client_fd) + "断开连接";

  this->client_fds.erase(client_fd);
  this->client_addrs.erase(client_fd);
  FD_CLR(client_fd, &this->fds);
  close(client_fd);

  std::cout << message << std::endl;
}

void ChatServer::sendPrivateMessage(int client_fd, babble::BabbleProtocol code, std::string message)
{
  babble::BabbleMessage msg;
  msg.message = message;
  msg.to = client_fd;
  msg.from = -1;
  msg.type = babble::BabbleType::PRIVATE;
  msg.code = code;
  babble::sendMessage(client_fd, msg);
  return;
}

void ChatServer::sendBroadcastMessage(babble::BabbleProtocol code, std::string message, const std::set<int> &group)
{
  babble::BabbleMessage msg;
  msg.message = message;
  msg.from = -1;
  msg.type = babble::BabbleType::BROAD;
  msg.code = code;
  for (int fd : group)
  {
    msg.to = fd;
    babble::sendMessage(fd, msg);
  }
}

int ChatServer::getOnlineCount()
{
  return this->client_fds.size() - std::count(this->client_fds.begin(), this->client_fds.end(), 0);
}

int ChatServer::getGroupOnlineCount(int group_id)
{
  return this->client_groups[group_id].size();
}

std::string ChatServer::getClientName(int client_fd)
{
  struct sockaddr_in &client_address = this->client_addrs[client_fd];
  std::string name = "客户端" + std::string(inet_ntoa(client_address.sin_addr)) + std::to_string(ntohs(client_address.sin_port)) + " fd:" + std::to_string(client_fd);
  return name;
}

json ChatServer::getOnlineList()
{
  json arr;
  for (int fd : this->client_fds)
  {
    std::string name = this->getClientName(fd);
    json obj;
    obj["socket"] = fd;
    obj["name"] = name;
    arr.push_back(obj);
  }
  return arr;
}