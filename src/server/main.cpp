#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <thread>
#include <mutex>
#include <iostream>
#include <vector>

#define MAX_LEN 200
#define NUM_COLORS 6
#define BUFFSIZE 2048
#define DEFAULT_PORT 16555

int client_socket, server_socket;

void stopServerRunning(int p)
{
  close(server_socket);
  printf("Close Server\n");
  exit(0);
}

int main()
{
  std::cout << "Hello World! Server"
            << "\n";
  struct sockaddr_in server_address;
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1)
  {
    perror("Socket creation failed");
    exit(1);
  }

  // bzero() 会将内存块（字符串）的前n个字节清零;
  bzero(&server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(DEFAULT_PORT);

  if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
  {
    perror("Bind failed");
    exit(1);
  }

  if (listen(server_socket, 2048) == -1)
  {
    perror("Listen failed");
    exit(1);
  }

  struct sockaddr_in client_addr;
  char buff[BUFFSIZE];

  std::cout << "listening...."
            << "\n";

  while (true)
  {
    signal(SIGINT, stopServerRunning);
    client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == -1)
    {
      perror("Accept failed");
      exit(1);
    }
    // END
    bzero(buff, BUFFSIZE);
    // 对应伪代码中的recv(connfd, buff);
    recv(client_socket, buff, BUFFSIZE - 1, 0);
    // END
    printf("Recv: %s\n", buff);
    // 对应伪代码中的send(connfd, buff);
    send(client_socket, buff, strlen(buff), 0);
    // END
    // 对应伪代码中的close(connfd);
    close(client_socket);
  }

  close(server_socket);
  return 0;
}
