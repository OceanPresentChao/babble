#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <thread>
#include <mutex>
#include <iostream>
#include <vector>

#define MAX_LEN 200
#define DEFAULT_PORT 16555
#define BUFFSIZE 2048

int main()
{
  int client_socket;
  struct sockaddr_in server_address;

  std::cout << "Hello World! Client"
            << "\n";
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket == -1)
  {
    printf("Create socket error(%d): %s\n", errno, strerror(errno));
    exit(1);
  }

  // bzero() 会将内存块（字符串）的前n个字节清零;
  bzero(&server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);
  server_address.sin_port = htons(DEFAULT_PORT);

  if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
  {
    printf("Connect error(%d): %s\n", errno, strerror(errno));
    exit(1);
  }
  char buff[BUFFSIZE];
  printf("Please input: ");
  scanf("%s", buff);
  send(client_socket, buff, strlen(buff), 0);
  bzero(buff, sizeof(buff));
  recv(client_socket, buff, BUFFSIZE - 1, 0);
  printf("Recv: %s\n", buff);
  close(client_socket);

  return 0;
}
