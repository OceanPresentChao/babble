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
    EXIT = 3,
    NEW_SESS = 4,
    MESSAGE = 5,
    QUERY = 6,
    INVALID = 7
  };

  enum BabbleType
  {
    BROAD = -1,
    ONE = -2,
    SERVER = -3,
  };

  const unsigned int BabblePkgWidth = sizeof(int);

  struct BabblePackage
  {
    int length;
    char message[BUFFSIZE];
  };

  struct BabbleMessage
  {
    BabbleProtocol code;
    BabbleType type;
    std::string message;
    int from;
    int to;
  };

  std::string formatMessage(struct BabbleMessage message);

  json parseMessage(std::string message);

  int recvMessage(int client_fd, std::string &message);

  int sendMessage(int client_fd, struct BabbleMessage message);

  int recvN(int client_fd, char *buffer, int length);

  int sendN(int client_fd, char *buffer, int length);

  json loadConfig(std::string configfile);

} // namespace babble
