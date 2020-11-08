#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#define ERROR (printf("[ERROR]\n%s: %d\n", __func__, __LINE__))

int main(int argc, char *argv[])
{
  int sock0;
  struct sockaddr_in addr;
  struct sockaddr_in client;
  socklen_t len;
  int sock;
  int fd;
  int n, ret;
  char buf[65536];

  if (argc != 1)
  {
    fprintf(stderr, "Usage : %s outputfilename\n", argv[0]);
    return 1;
  }

  // fd = open(argv[1], O_WRONLY | O_CREAT, 0600);
  if (fd < 0)
  {
    perror("open");
    return 1;
  }

/* ソケットの作成 */
// #define SOCKET_ERROR
#ifndef SOCKET_ERROR
  sock0 = socket(AF_INET, SOCK_STREAM, 0); //数字に意味はない
#else
  sock0 = socket(3000, 4000, 5000); //数字に意味はない
#endif
  if (sock0 < 0)
  {
    perror("socket");
    printf("%d\n", errno);
    return 1;
  }

  /* ソケットの設定 */
  addr.sin_family = AF_INET;
  addr.sin_port = htons(12345);
  addr.sin_addr.s_addr = INADDR_ANY;
// #define BIND_ERROR
#ifndef BIND_ERROR
  int bind_error = bind(sock0, (struct sockaddr *)&addr, sizeof(addr));
#else
  // int bind_error = bind(sock, (struct sockaddr *)&addr, 50);
  int bind_error = bind(sock0, (struct sockaddr *)&addr, sizeof(addr));

#endif
  if (bind_error < 0)
  {
    perror("bind");
    printf("%d\n", errno);
    close(sock);
    close(sock0);
    return 1;
  }

  /* TCPクライアントからの接続要求を待てる状態にする */
  // #define LISTEN_ERROR
#ifndef LISTEN_ERROR
  int listen_error = listen(sock0, 5);
#else
  int listen_error = listen(300, 5);
#endif
  if (listen_error < 0)
  {
    perror("listn");
    printf("%d\n", errno);
    close(sock);
    close(sock0);
    return 1;
  }
  /* TCPクライアントからの接続要求を受け付ける */
  len = sizeof(client);
// #define ACCEPT_ERROR
#ifndef ACCEPT_ERROR
  sock = accept(sock0, (struct sockaddr *)&client, &len);
#else
  sock = accept(300, (struct sockaddr *)&client, &len);
#endif
  if (sock < 0)
  {
    perror("accept");
    printf("%d\n", errno);
    close(sock);
    close(sock0);
    return 1;
  }

  char filename[512] = {'0'};
  // ファイル名受信
  n = read(sock, filename, sizeof(filename));
  printf("filename: %s\n", filename);
  // ファイルオープン
  fd = open(filename, O_RDONLY);
  printf("fd: %d\n", fd);
  if (fd < 0)
  {
    perror("指定されたファイルは存在しません");
    if ((write(sock, "no_file", 7)) < 0)
    {
      ERROR;
      perror("write");
      close(sock);
      close(sock0);
      return 1;
    }
    close(sock);
    close(sock0);
    return 1;
  }
  else
  {
    if ((write(sock, "do_next", 7)) < 0)
    {
      ERROR;
      perror("write");
      close(sock);
      close(sock0);
      return 1;
    }
  }
  memset(buf, 0, sizeof(buf));
  if ((read(sock, buf, sizeof(buf))) < 0)
  {
    ERROR;
    perror("read");
    close(sock);
    close(sock0);
    return 0;
  }
  if (strlen(buf) == 5 && strstr(buf, "start") != NULL)
  {
    puts("ファイル送信を開始します");
    // ファイル送信
    while ((n = read(fd, buf, sizeof(buf))) > 0) //ファイル読み込み
    {
      // printf("n: %d\n", n);
      ret = write(sock, buf, n); //ファイル送信
      if (ret < 1)
      {
        perror("write");
        break;
      }
    }
  }

  /* TCPセッションの終了 */
  close(sock);

  /* listen するsocketの終了 */
  close(sock0);

  return 0;
}
