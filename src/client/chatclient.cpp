#include "ChatClient.h"
#include "../common/ChatRoom.h"
#include <iostream>
#include <stdexcept>

ChatClient::ChatClient()
{
  this->isRunning = false;
  this->status = ClientStatus::IDLE;
}

ChatClient::~ChatClient()
{
  this->disconnect();
}

void ChatClient::setHost(std::string host)
{
  this->host = host;
}

void ChatClient::setPort(int port)
{
  this->port = port;
}

void ChatClient::init()
{
  // bzero() 会将内存块（字符串）的前n个字节清零;
  bzero(&this->server_address, sizeof(this->server_address));
  this->server_address.sin_family = AF_INET;
  inet_pton(AF_INET, this->host.c_str(), &this->server_address.sin_addr);
  this->server_address.sin_port = htons(this->port);
}

int ChatClient::connectServer()
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

  // 连接成功以后，我们再将 clientfd 设置成非阻塞模式，
  // 不能在创建时就设置，这样会影响到 connect 函数的行为
  int oldSocketFlag = fcntl(this->ct_socket, F_GETFL, 0);
  int newSocketFlag = oldSocketFlag | O_NONBLOCK;
  if (fcntl(this->ct_socket, F_SETFL, newSocketFlag) == -1)
  {
    this->disconnect();
    std::cout << "set socket to nonblock error." << std::endl;
    return -1;
  }

  return 0;
}

int ChatClient::disconnect()
{
  this->isRunning = false;
  close(this->ct_socket);
  return 0;
}

int ChatClient::receivePackage(babble::BabblePackage &pkg)
{
  while (this->isRunning)
  {
    int ret = babble::recvPackage(this->ct_socket, pkg);
    if (ret > 0)
    {
      // 收到了数据
      // std::cout << "recv successfully." << std::endl;
    }
    else if (ret == 0)
    {
      // 对端关闭了连接
      std::cout << "server close the socket." << std::endl;
      return 0;
    }
    else if (ret == -1)
    {
      if (errno == EWOULDBLOCK)
      {
        // std::cout << "There is no data available now." << std::endl;
        continue;
      }
      else if (errno == EINTR)
      {
        // 如果被信号中断了，则继续重试recv函数
        std::cout << "recv data interrupted by signal." << std::endl;
        continue;
      }
      else
      {
        // 真的出错了
        throw "recv data error.";
        break;
      }
    }
    return ret;
  }
  return -1;
}

int ChatClient::sendPackage(babble::BabblePackage package)
{
  while (this->isRunning)
  {
    int ret = babble::sendPackage(this->ct_socket, package);
    if (ret == -1)
    {
      // 非阻塞模式下send函数由于TCP窗口太小发不出去数据，错误码是EWOULDBLOCK
      if (errno == EWOULDBLOCK)
      {
        std::cout << "send data error as TCP Window size is too small." << std::endl;
        continue;
      }
      else if (errno == EINTR)
      {
        // 如果被信号中断
        std::cout << "sending data interrupted by signal." << std::endl;
        continue;
      }
      else
      {
        throw "send data error.";
      }
    }
    if (ret == 0)
    {
      // 对端关闭了连接
      return 0;
    }
    return ret;
  }
  return -2;
}

void ChatClient::run()
{
  std::cout << "client run" << std::endl;
  this->isRunning = true;
  while (this->isRunning)
  {
    system("clear");
    std::cout << "1.进入全局聊天" << std::endl;
    std::cout << "2.开始私聊" << std::endl;
    std::cout << "3.退出" << std::endl;

    int send_num = 0;
    std::string buff;
    babble::BabblePackage pkg;

    buff.clear();
    std::cin >> buff;

    switch (buff[0])
    {
    case '1':
    {
      pkg.header.type = babble::BabbleType::JOIN;
      std::cout << "请输入聊天室序号:";
      buff.clear();
      std::cin >> buff;
      int group_id = -1;
      if (buff == "#exit")
      {
        break;
      }
      try
      {
        group_id = std::stoi(buff);
      }
      catch (std::invalid_argument &)
      {
        std::cout << "非法序号！" << std::endl;
        break;
      }
      pkg.body.at("message") = buff;
      try
      {
        this->sendPackage(pkg);
      }
      catch (std::string &e)
      {
        std::cout << e << std::endl;
        break;
      }
      this->receivePackage(pkg);
      if (pkg.header.status == babble::BabbleStatus::INVALID)
      {
        std::cout << "Invalid:" << pkg.body.at("message") << std::endl;
      }
      else
      {
        this->handleGroupChat(group_id);
      }
      break;
    }
    case '3':
    {
      break;
    }
    default:
      break;
    }
  }
  this->disconnect();
  return;
}

void ChatClient::handleReceiveChat(void *client)
{
  ChatClient *chatClient = (ChatClient *)client;
  babble::BabblePackage pkg;
  while (chatClient->isRunning && chatClient->status == ClientStatus::CHATTING)
  {
    chatClient->receivePackage(pkg);
    std::string message = pkg.body.at("message");
    babble::BabbleType type = pkg.header.type;
    if (type == babble::BabbleType::MESSAGE)
    {
      std::cout << message << std::endl;
    }
  }
}

void ChatClient::handleGroupChat(int group_id)
{
  babble::BabblePackage pkg;
  pkg.header.type = babble::BabbleType::MESSAGE;
  std::string raw = "";

  this->status = ClientStatus::CHATTING;
  std::thread t1(ChatClient::handleReceiveChat, (void *)this);
  this->recv_thread = std::move(t1);
  this->recv_thread.detach();

  pkg.header.to = group_id;

  while (this->isRunning && this->status == ClientStatus::CHATTING)
  {
    std::cout << "请输入：" << std::endl;
    raw.clear();
    std::cin >> raw;
    pkg.body.at("message") = raw;
    if (raw == "#exit")
    {
      pkg.header.type = babble::BabbleType::EXIT;
      this->status = ClientStatus::IDLE;
    }
    this->sendPackage(pkg);
  }
}
