#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// #define MESSAGE "HELLO"

char *get_adress(char *hostname)
{
  struct addrinfo hints, *res;
  struct in_addr addr;
  char *buf = (char *)malloc(sizeof(char) * 16);
  if (buf != NULL)
  {
    memset(buf, '\0', 16);
  }
  int err;

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;
  if ((err = getaddrinfo(hostname, NULL, &hints, &res)) != 0)
  {
    printf("error %d\n", err);
    return NULL;
  }

  addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
  inet_ntop(AF_INET, &addr, buf, 16);
  printf("ip address : %s\n", buf);

  freeaddrinfo(res);
  return buf;
}

void print_addr(struct sockaddr_in addr)
{
  printf("\nport: %d\n", addr.sin_port);
  printf("adderss: %d\n\n", addr.sin_addr.s_addr);
}

int main(int argc, char *argv[])
{
  int sock;
  struct sockaddr_in addr;
  int n;
  char hostname[32];
  int port_num = 0;
  char *temp;
  if (argc == 3)
  {
    strcpy(hostname, argv[1]);
    port_num = atoi(argv[2]);
    if (hostname[0] < '0' || hostname[0] > '9')
    {
      temp = get_adress(hostname);
      strcpy(hostname, temp);
    }
    free(temp);
    port_num = atoi(argv[2]);
  }
  else
  {
    strcpy(hostname, "localhost");
    port_num = 12345;
  }

  /* ���s��F ./simpleUDPsender 127.0.0.1 */
  if (argc != 3)
  {
    fprintf(stderr, "Usage : %s dstipaddr\n", argv[0]);
    return 1;
  }

  // 送信
  // bindするソケットとしないソケットで分ける
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  int sock_recv = socket(AF_INET, SOCK_DGRAM, 0);

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_num);
  inet_pton(AF_INET, hostname, &addr.sin_addr.s_addr);
  print_addr(addr);
  // puts("send->HELLO");
  printf("送るメッセージを入力してください\n>");
  char message[2048] = {'\0'};
  scanf("%s", message);
  printf("send->%s\n", message);
  n = sendto(sock, message, 5, 0, (struct sockaddr *)&addr, sizeof(addr));
  if (n < 1)
  {
    perror("[error]\nsendto");
    return 1;
  }

  // 受信
  char buf[2048];
  socklen_t addrlen;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(23456);
  if (bind(sock_recv, (struct sockaddr *)&addr, sizeof(addr)) == -1)
  {
    perror("bind");
    return 1;
  }

  addrlen = sizeof(addr);
  puts("wait recieve");
  print_addr(addr);
  n = recvfrom(sock_recv, buf, sizeof(buf) - 1, 0,
               (struct sockaddr *)&addr, &addrlen);
  if (n < 1)
  {
    perror("[reciever]\nrecvfrom");
    return 1;
  }
  printf("recieve<- %s\n", buf);

  close(sock);
  close(sock_recv);

  return 0;
}
