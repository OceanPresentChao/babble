#include "ChatClient.h"
#include "../common/chatroom.h"
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

  // 获取文件标识符
  int flags = fcntl(this->ct_socket, F_GETFL, 0);
  if (flags < 0)
  {
    perror("fcntl failed");
    return -1;
  }
  // 设置非阻塞标志：在获取标志后，将 O_NONBLOCK 标志设置到 flags 中
  flags |= O_NONBLOCK;
  // 更新文件描述符的标志：使用 fcntl 函数更新文件描述符的标志。
  int ret = fcntl(this->ct_socket, F_SETFL, flags);
  if (ret < 0)
  {
    perror("fcntl failed");
    return -1;
  }

  if (connect(this->ct_socket, (struct sockaddr *)&this->server_address, sizeof(this->server_address)) == -1)
  {
    printf("Connect error(%d): %s\n", errno, strerror(errno));
    return -1;
  }
  return 0;
}

int ChatClient::disconnect()
{
  close(this->ct_socket);
  return 0;
}

json ChatClient::receiveMessage()
{
  std::string recvMsg;
  int num_recv = babble::recvMessage(this->ct_socket, recvMsg);
  if (num_recv <= 0)
  {
    std::cout << "Tip: No Message Received!" << std::endl;
  }
  return babble::parseMessage(recvMsg);
}

int ChatClient::sendMessage(babble::BabbleMessage message)
{
  return babble::sendMessage(this->ct_socket, message);
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
    std::string buff;
    buff.clear();
    std::cin >> buff;
    switch (buff[0])
    {
    case '1':
    {
      babble::BabbleMessage msg(babble::BabbleProtocol::JOIN, babble::BabbleType::SERVER);
      std::cout << "请输入聊天室序号:";
      buff.clear();
      std::cin >> buff;
      if (buff == "#exit")
      {
        break;
      }
      try
      {
        std::stoi(buff);
      }
      catch (std::invalid_argument &)
      {
        std::cout << "非法序号！" << std::endl;
        break;
      }
      msg.message = buff;
      this->sendMessage(msg);
      json recvMsg = this->receiveMessage();
      if (recvMsg["code"].get<int>() == babble::BabbleProtocol::INVALID)
      {
        std::cout << "Invalid:" << recvMsg["message"].get<std::string>() << std::endl;
      }
      else
      {
        this->handleGroupChat();
      }
      break;
    }
    case '2':
    {

      break;
    }
    case '3':
    {
      this->isRunning = false;
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
  while (chatClient->isRunning)
  {
    json j = chatClient->receiveMessage();
    std::string message = j["message"].get<std::string>();
    std::cout << message << std::endl;
  }
}

void ChatClient::handleGroupChat()
{
  babble::BabbleMessage msg(babble::BabbleProtocol::MESSAGE, babble::BabbleType::BROAD);
  std::string raw = "";

  this->status = ClientStatus::CHATTING;
  std::thread t1(ChatClient::handleReceiveChat, (void *)this);
  this->recv_thread = std::move(t1);
  this->recv_thread.detach();

  while (this->isRunning && this->status == ClientStatus::CHATTING)
  {
    std::cout << "请输入：" << std::endl;
    raw.clear();
    std::cin >> raw;
    msg.message = raw;
    if (raw == "#exit")
    {
      msg.code = babble::BabbleProtocol::LOGOUT;
      msg.type = babble::BabbleType::SERVER;
      break;
    }
    this->sendMessage(msg);
  }
  this->status = ClientStatus::IDLE;
}

void ChatClient::handlePrivateChat()
{
}