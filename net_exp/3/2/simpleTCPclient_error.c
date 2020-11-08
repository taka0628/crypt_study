#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

int main()
{
  struct sockaddr_in server;
  int sock;
  char buf[32];
  int n;

  /* ソケットの作成 */
  sock = socket(AF_INET, SOCK_STREAM, 0);

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
