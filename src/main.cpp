#include "../include/hello.h"
#include <string>
int main()
{
  hello();
  std::string name;
  std::cin >> name;
  std::cout << name;
  return 0;
}