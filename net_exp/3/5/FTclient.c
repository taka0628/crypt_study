#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

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

int main(int argc, char *argv[])
{
  struct sockaddr_in server;
  int sock;
  int fd;
  char buf[65536];
  int n, ret;

  if (argc != 3)
  {
    fprintf(stderr, "Usage : %s hostname\n", argv[0]);
    return 1;
  }

  fd = open(argv[2], O_RDONLY);
  if (fd < 0)
  {
    perror("open");
    return 1;
  }

  /* ソケットの作成 */
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror("socket");
    printf("%d\n", errno);
    return 1;
  }

  /* 接続先指定用構造体の準備 */
  server.sin_family = AF_INET;
  server.sin_port = htons(12345);

  /* 127.0.0.1はlocalhost */
  char *temp = get_adress(argv[1]);
  char addr[32] = {'\0'};
  if(temp != NULL){
    strcpy(temp, addr);
    free(temp);
    inet_pton(AF_INET, addr, &server.sin_addr.s_addr);
  }else{
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr.s_addr);
  }

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
  
// ファイル送信
  while ((n = read(fd, buf, sizeof(buf))) > 0)// ファイル読み込み
  {
    // printf("n: %d\n", n);
    ret = write(sock, buf, n);//ファイル送信
    if (ret < 1)
    {
      perror("write");
      break;
    }
  }
  /* socketの終了 */
  close(sock);

  return 0;
}
