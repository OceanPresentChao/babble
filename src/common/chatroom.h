#ifndef CHATROOM_H
#define CHATROOM_H
#include <fstream>
#include "json.hpp"
#include <string>
#include <sys/socket.h>

#define DEFAULT_PORT 16554
#define BUFFSIZE 2048

using json = nlohmann::json;

namespace babble
{

  enum BabbleStatus
  {
    INVALID = 0,
    OK = 1
  };

  enum BabbleType
  {
    LOGIN = 0,
    LOGOUT = 1,
    JOIN = 2,
    EXIT = 3,
    NEW_SESS = 4,
    CLOSE_SESS = 5,
    MESSAGE = 6,
    QUERY = 7,
    RESPONSE = 8
  };

  struct BabbleHeader
  {
    int from;
    int to;
    BabbleStatus status;
    BabbleType type;
    // 算上包头的总长度
    int length;
    int padding;
    BabbleHeader() : from(0), to(0), length(0){};
    BabbleHeader(int from, int to, BabbleStatus status, BabbleType type, int length) : from(from), to(to), status(status), type(type), length(length){};
  };

  const unsigned int BabblePkgHdrWidth = sizeof(BabbleHeader);

  struct BabblePackage
  {
    struct BabbleHeader header;
    json body;
    BabblePackage()
    {
      body = json::parse(R"(
        {
          "message": ""
        }
      )");
    }
    BabblePackage(struct BabbleHeader header, json body) : header(header), body(body){};
  };

  int recvPackage(int client_fd, struct BabblePackage &package);

  int sendPackage(int client_fd, struct BabblePackage package);

  int recvN(int client_fd, char *buffer, int length);

  int sendN(int client_fd, char *buffer, int length);

  json loadConfig(std::string configfile);

} // namespace babble

#endif
