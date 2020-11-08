#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

int main(int argc, char *argv[])
{
  struct sockaddr_in server;
  int sock;
  int fd;
  char buf[65536];
  int n, ret;

  if (argc != 2)
  {
    fprintf(stderr, "Usage : %s filename\n", argv[0]);
    return 1;
  }

  fd = open(argv[1], O_RDONLY);
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
  inet_pton(AF_INET, "127.0.0.1", &server.sin_addr.s_addr);

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
  

  while ((n = read(fd, buf, sizeof(buf))) > 0)
  {
    printf("n: %d\n", n);
    ret = write(sock, buf, n);
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
