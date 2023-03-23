#include "chatclient.h"
#include "../common/chatroom.h"
#include <iostream>

ChatClient::ChatClient()
{
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
  return 0;
}

int ChatClient::disconnect()
{
  close(this->ct_socket);
  return 0;
}

int ChatClient::sendMessage(std::string message, int to)
{
  babble::BabbleMessage msg;
  msg.message = message;
  msg.type = babble::BabbleType::BROAD;
  msg.code = babble::BabbleProtocol::MESSAGE;
  msg.to = to;
  msg.from = this->ct_socket;
  babble::sendMessage(this->ct_socket, msg);
  return message.size();
}

void ChatClient::receiveMessage(int client_fd)
{
  while (true)
  {
    std::string message = "";
    int num_recv = babble::recvMessage(client_fd, message);
    if (num_recv <= 0)
    {
      std::cout << "Tip: No Message Received!" << std::endl;
      break;
    }
    std::cout << "num_recv: " << num_recv << std::endl;
    std::cout << "msg_recv: " << std::endl;
    std::cout << message << std::endl;
  }
}

void ChatClient::run()
{
  std::cout << "client run" << std::endl;
  std::thread t1(ChatClient::receiveMessage, this->ct_socket);
  this->recv_thread = std::move(t1);
  this->recv_thread.detach();
  while (true)
  {
    char buff[BUFFSIZE];
    bzero(buff, sizeof(buff));
    printf("Please input: ");
    scanf("%s", buff);
    std::string message(buff);
    this->sendMessage(message, -1);
    if (message == "#exit")
    {
      std::cout << "client exit" << std::endl;
      break;
    }
  }
  this->disconnect();
  return;
}