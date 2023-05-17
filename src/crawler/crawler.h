#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

// IP报文格式
typedef struct IP
{
  // unsigned char version;//4位IP版本号
  unsigned char headLen;     // 4位首部长度
  unsigned char serviceType; // 8位服务类型
  unsigned short totalLen;   // 16位总长度
  unsigned short identifier; // 16位标识符
  unsigned short flags;      // 3位标志位
  // unsigned short fragOffset;//13位片偏移
  unsigned char timeToLive;    // 8位生存时间
  unsigned char protocol;      // 8位协议
  unsigned short headCheckSum; // 16位首部校验和
  unsigned int sourceAddr;     // 32位源地址
  unsigned int destinAddr;     // 32位目的地址
} IPHeader;

// TCP报文格式
typedef struct TCP
{
  unsigned short sourcePort; // 16位源端口号
  unsigned short destinPort; // 16位目的端口号
  unsigned int seqNum;       // 32位序列号
  unsigned int ackNum;       // 32位确认号
  unsigned char headLen;     // 4位首部长度
  // unsigned char resv;//4位保留字
  unsigned char flags;       // 8位标志位
  unsigned short winSize;    // 16位窗口大小
  unsigned short checkNum;   // 16位校验和
  unsigned short urgPointer; // 16位紧急指针
} TCPHeader;
