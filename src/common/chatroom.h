#include <fstream>
#include "json.hpp"

#define MAX_LEN 200
#define DEFAULT_PORT 16554
#define BUFFSIZE 2048

using json = nlohmann::json;

inline json loadConfig(std::string configfile)
{
  std::ifstream f(configfile);
  return json::parse(f);
}