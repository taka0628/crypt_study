#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

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

  if (argc != 2) {
    fprintf(stderr, "Usage : %s outputfilename\n", argv[0]);
    return 1;
  }

  fd = open(argv[1], O_WRONLY | O_CREAT, 0600);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  /* ソケットの作成 */
  sock0 = socket(AF_INET, SOCK_STREAM, 0);
  
  /* ソケットの設定 */
  addr.sin_family = AF_INET;
  addr.sin_port = htons(12345);
  addr.sin_addr.s_addr = INADDR_ANY;
  bind(sock0, (struct sockaddr *)&addr, sizeof(addr));
  
  /* TCPクライアントからの接続要求を待てる状態にする */
  listen(sock0, 5);

  /* TCPクライアントからの接続要求を受け付ける */
  len = sizeof(client);
  sock = accept(sock0, (struct sockaddr *)&client, &len);

  while ((n = read(sock, buf, sizeof(buf))) > 0) {
    ret = write(fd, buf, n);
    if (ret < 1) {
      perror("write");
      break;
    }
  }

  /* TCPセッションの終了 */
  close(sock);

  /* listen するsocketの終了 */
  close(sock0);

  return 0;
}
