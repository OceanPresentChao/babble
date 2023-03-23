#pragma once
#include <fstream>
#include "json.hpp"
#include <string>
#include <sys/socket.h>

#define DEFAULT_PORT 16554
#define BUFFSIZE 2048

using json = nlohmann::json;

namespace babble
{

  enum BabbleProtocol
  {
    LOGIN = 0,
    LOGOUT = 1,
    JOIN = 2,
    LEAVE_SESS = 3,
    NEW_SESS = 4,
    MESSAGE = 5,
    QUERY = 6,
    INVALID = 7
  };

  const int BabblePkgWidth = sizeof(int);

  struct BabblePackage
  {
    int length;
    char message[BUFFSIZE];
  };

  std::string formatMessage(enum BabbleProtocol type, int code, std::string message);

  json parseMessage(std::string message);

  int recvMessage(int client_fd, std::string &message);

  int sendMessage(int client_fd, babble::BabbleProtocol type, int code, std::string message);

  json loadConfig(std::string configfile);

} // namespace babble
