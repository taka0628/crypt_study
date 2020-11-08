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

typedef enum
{
  false,
  true
} bool;

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

int get_option_cmd(char *filename)
{
  // オプション入力
  int option = 0;
  char buff_option[512] = {'\0'};
  printf("受信するファイル名を変更しますか？変更する->1, 変更しない->0\n");
  while (1)
  {
    printf(">");
    if (fgets(buff_option, 512, stdin) == NULL)
    {
      puts("option fgets error");
      printf("buff_option: %s", buff_option);
      return 1;
    }
    if (buff_option[strlen(buff_option)] == '\n')
    {
      buff_option[strlen(buff_option)] == '0';
    }
    if (strlen(buff_option) > 1)
    {
      puts("文字数が多すぎます");
    }
    if (strlen(buff_option) == 1 && (buff_option[0] == '0' || buff_option[0] == '1'))
    {
      break;
    }
  }
  int result = atoi(buff_option[0]);
  for (int i = 0; i < strlen(buff_option); i++)
  {
    buff_option[i] = '\0';
  }
  if (result == 1)
  {
    printf("ファイル名を入力してください\n");
    printf(">");
    if (fgets(buff_option, 512, stdin) == NULL)
    {
      puts("option fgets error");
      printf("buff_option: %s", buff_option);
      return 1;
    }
    if (buff_option[strlen(buff_option)] == '\n')
    {
      buff_option[strlen(buff_option)] == '0';
    }
  }
  strcpy(filename, buff_option);
  return result;
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

  // fd = open(argv[1], O_RDONLY, O_CREAT, O_EXCL);
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
  if (temp != NULL)
  {
    strcpy(temp, addr);
    free(temp);
    inet_pton(AF_INET, addr, &server.sin_addr.s_addr);
  }
  else
  {
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

  n = write(sock, argv[2], strlen(argv[2]));

  char filename[128] = {'\0'};
  char *temp_name;
  temp_name = strtok(argv[2], "/");
  while (temp_name != NULL)
  {
    strcpy(filename, temp_name);
    temp_name = strtok(NULL, "/");
  }
  printf("clientの設定したファイル名は%sです\n", filename);

  if ((fd = open(filename, O_RDONLY)) < 0)
  {
    puts("同じ名前のファイルは存在していません");
  }
  else
  {
    puts("同じ名前のファイルが存在しています");
    close(fd);
  }
  int isRename = get_option_cmd(filename);

  fd = open(filename, O_WRONLY | O_CREAT, 0600);
  // fd = open(filename, O_WRONLY);
  if (fd < 0)
  {
    perror("open");
    return 1;
  }
  while ((n = read(sock, buf, sizeof(buf))) > 0)
  {
    // puts("[server]\nloop");
    ret = write(fd, buf, n);
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
