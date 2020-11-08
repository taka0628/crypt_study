#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define ADRESS
#ifdef ADRESS
char* get_adress(char *hostname)
{
  struct addrinfo hints, *res;
  struct in_addr addr;
  char* buf = (char *)malloc(sizeof(char) * 16);
  if(buf != NULL){
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
#endif

int main(int argc, char *argv[])
{
  struct sockaddr_in server;
  int sock;
  char buf[32];
  int n;
  char hostname[32];
  char adress[32];
  int port_num = 0;

  puts("main st");
  if (argc == 3)
  {
    puts("argc == 3 st");
    strcpy(hostname, argv[1]);
    printf("hostname: %s\n", hostname);
    port_num = atoi(argv[2]);
    printf("port_num: %d\n", port_num);
#ifndef ADRESS
    struct addrinfo hints, *res;
    struct in_addr addr;
    char buf[16];
    int err;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    if ((err = getaddrinfo(hostname, NULL, &hints, &res)) != 0)
    {
      printf("error %d\n", err);
      return 1;
    }

    addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
    inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    printf("ip address : %s\n", buf);

    freeaddrinfo(res);
    strcpy(adress, buf);
#else
    char *temp = get_adress(hostname);
    strcpy(adress, temp);
    free(temp);
#endif
    printf("adress: %s\n", adress);
  }
  else
  {
    strcpy(hostname, "localhost");
    port_num = 12345;
    strcpy(adress, "127.0.0.1");
  }
  puts("adress comp");

  /* ソケットの作成 */
  sock = socket(AF_INET, SOCK_STREAM, 0);

  /* 接続先指定用構造体の準備 */
  server.sin_family = AF_INET;
  server.sin_port = htons(port_num);

  /* 127.0.0.1はlocalhost */
  inet_pton(AF_INET, adress, &server.sin_addr.s_addr);

  /* サーバに接続 */
// #define CONNECT_ERROR
#ifndef CONNECT_ERROR
  int connect_error = connect(sock, (struct sockaddr *)&server, sizeof(server));
#else
  int connect_error = connect(sock, (struct sockaddr *)&server, -1);
#endif
  if (connect_error < 0)
  {
    perror("connect");
    printf("%d\n", errno);
    return 1;
  }

  /* サーバからデータを受信 */
  memset(buf, 0, sizeof(buf));
// #define READ_ERROR
#ifndef READ_ERROR
  n = read(sock, buf, sizeof(buf));
#else
  n = read(sock, buf, -1);
#endif

  if (n < 0)
  {
    perror("read");
    printf("%d\n", errno);
    close(sock);
    return 1;
  }

  printf("%d, %s\n", n, buf);

  /* socketの終了 */
  close(sock);

  return 0;
}
