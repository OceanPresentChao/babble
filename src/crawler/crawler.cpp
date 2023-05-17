#include "crawler.h"
#include <cstring>
#include <iostream>

int main(int argc, char **argv)
{

  int sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
  if (sock == -1)
  {
    throw "Failed to create socket";
  }

  // int enable = 1;
  // if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &enable, sizeof(enable)) < 0)
  // {
  //   // 处理设置套接字选项失败的情况
  //   return -1;
  // }

  struct sockaddr_in sourceAddress;
  bzero(&sourceAddress, sizeof(sourceAddress));
  socklen_t addressSize = sizeof(sourceAddress);
  // sourceAddress.sin_family = AF_INET;
  // inet_pton(AF_INET, "127.0.0.1", &sourceAddress.sin_addr.s_addr);
  // sourceAddress.sin_port = htons(12345);

  bind(sock, (struct sockaddr *)&sourceAddress, sizeof(sourceAddress));

  while (true)
  {
    std::cerr << "listen";
    char buffer[50]; // 缓冲区用于接收数据包
    memset(buffer, 0, sizeof(buffer));

    ssize_t packetSize = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&sourceAddress, &addressSize);
    if (packetSize < 0)
    {
      // 处理接收数据包失败的情况
      return -1;
    }

    // 在此处对接收到的数据包进行处理，可以分析 TCP 头部等信息
    IP *ip;
    TCP *tcp;
    std::cerr << "receivedLength: " << packetSize << std::endl;
    ip = (struct IP *)buffer;
    ip->headLen = (ip->headLen & 0x0f) * 4;

    if (ip->protocol == 6)
    {
      tcp = (struct TCP *)(buffer + sizeof(IP) + ip->headLen - 20);
      // cout << endl <<  sizeof(IP)<< endl;
      std::cout << "Network+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
      tcp->headLen = (tcp->headLen >> 4) * 4;
      char ip_src[INET_ADDRSTRLEN], ip_dst[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &(ip->sourceAddr), ip_src, INET_ADDRSTRLEN);
      inet_ntop(AF_INET, &(ip->destinAddr), ip_dst, INET_ADDRSTRLEN);
      std::cout << "源IP：" << ip_src << "\n";
      std::cout << "目的IP：" << ip_dst << "\n";
      std::cout << "Transportation++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
      std::cout << "源端口：" << ntohs(tcp->sourcePort) << "\n";
      std::cout << "目的端口：" << ntohs(tcp->destinPort) << "\n";
      std::cout << "Applications++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
      char *start = buffer + sizeof(IP) + sizeof(TCP) + ip->headLen + tcp->headLen - 40;
      int dataLength = packetSize - (sizeof(IP) + sizeof(TCP) + ip->headLen + tcp->headLen - 40);
      std::cout << "数据内容：";
      memcpy(buffer, start, dataLength);
      if (dataLength == 0)
        std::cout << "[无]";
      for (int i = 0; i < dataLength; i++)
      {
        printf("%c", (buffer[i] >= 32 && buffer[i] < 255) ? (unsigned char)buffer[i] : '.');
      }
      puts("");
      puts("");
    }
  }

  close(sock);

  return 0;
}