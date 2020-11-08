#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void print_addr(struct sockaddr_in addr){
  printf("\nport: %d\n", addr.sin_port);
  printf("adderss: %d\n\n", addr.sin_addr.s_addr);
}

int main()
{
  int sock;
  struct sockaddr_in addr;
  struct sockaddr_in senderinfo;
  socklen_t addrlen;
  char buf[2048];
  int n;

  // 受信
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  int sock_send = socket(AF_INET, SOCK_DGRAM, 0);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(12345);
  addr.sin_addr.s_addr = INADDR_ANY;

  if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1){
    perror("bind");
    return 1;
  }
  addrlen = sizeof(senderinfo);
  print_addr(addr);
  n = recvfrom(sock, buf, sizeof(buf) - 1, 0,
               (struct sockaddr *)&senderinfo, &addrlen);
  printf("recieve<- %s\n", buf);
  if (n < 1)
  {
    perror("[reciever]\nrecvfrom");
    return 1;
  }

  // 送信
  addr.sin_addr.s_addr = senderinfo.sin_addr.s_addr;
  addr.sin_port = htons(23456);
  // inet_nton(AF_INET, "172.0.0.1", &addr.sin_addr.s_addr, sizeof(addr));
  print_addr(addr);
  sleep(1);
  printf("send-> %s\n", buf);
  n = sendto(sock_send, buf, strlen(buf), 0, (struct sockaddr *)&addr, sizeof(addr));
  if (n < 1)
  {
    perror("[reciever]\nsendto");
    return 1;
  } 

  close(sock);
  close(sock_send);

  return 0;
}
