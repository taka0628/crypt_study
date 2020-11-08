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

#define ERROR (printf("[ERROR]\n%s: %d\n", __func__, __LINE__))

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

// 1~option_sizeまでの範囲で入力がされているかの確認を行う
int safe_entry(int const option_size)
{
  char buff_option[512] = {'\0'};
  char *p = NULL;
  int result = 0;
  while (1)
  {
    memset(buff_option, 0, sizeof(buff_option));
    printf(">");
    if (fgets(buff_option, 512, stdin) == NULL)
    {
      puts("option fgets error");
      printf("buff_option: %s", buff_option);
      return 1;
    }
    // 改行文字を削除
    if ((p = strchr(buff_option, '\n')) != NULL)
    {
      // *p = '\0';
      p = NULL;
    }

    if (strlen(buff_option) > 2)
    {
      puts("文字数が多すぎます");
    }
    if (strlen(buff_option) == 2 && (buff_option[0] >= '0' || buff_option[0] <= (char)option_size))
    {
      result = atoi(buff_option);
      break;
    }
  }
  return result;
}

int get_option_cmd(char *filename)
{
  // オプション入力
  int option = 0;
  char buff_option[512] = {'\0'};
  printf("受信するファイル名を変更しますか？変更する->1, 変更しない->0\n");

  int result = safe_entry(1);

  if (result == 1)
  {
    memset(buff_option, 0, sizeof(buff_option));
    printf("ファイル名を入力してください\n");
    printf(">");
    if (fgets(buff_option, 512, stdin) == NULL)
    {
      ERROR;
      puts("option fgets error");
      printf("buff_option: %s", buff_option);
      return 1;
    }
    char *p;
    if ((p = strchr(buff_option, '\n')) != NULL)
    {
      *p = '\0';
      p = NULL;
    }
  }
  // 拡張子が入力されていなければ自動でつける
  char *p;
  if ((strstr(buff_option, ".")) == NULL)
  {
    p = strstr(filename, ".");
    sprintf(buff_option, "%s%s", buff_option, p);
  }
  // printf("buff_option: %s\n", buff_option);
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

  if (argc != 4)
  {
    ERROR;
    puts("./~ hostname file_path save_path");
    return 1;
  }

  // fd = open(argv[1], O_RDONLY, O_CREAT, O_EXCL);
  if (fd < 0)
  {
    ERROR;
    perror("open");
    return 1;
  }

  /* ソケットの作成 */
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    ERROR;
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
  int connect_error = connect(sock, (struct sockaddr *)&server, sizeof(server));
  if (connect_error < 0)
  {
    ERROR;
    perror("connect");
    printf("%d\n", errno);
    return 1;
  }

  char filename[128] = {'\0'};

  if (write(sock, argv[2], strlen(argv[2])) < 0)
  {
    ERROR;
    perror("write");
    close(sock);
  }

  if ((read(sock, buf, sizeof(buf))) < 0)
  {
    ERROR;
    perror("read");
    close(sock);
    return 0;
  }
  if (strlen(buf) == 7 && strstr(buf, "no_file") != NULL)
  {
    puts("指定されたファイルは存在しません");
    close(sock);
    return 1;
  }
  memset(buf, 0, sizeof(buf));

  char *temp_name;
  temp_name = strtok(argv[2], "/");
  while (temp_name != NULL)
  {
    strcpy(filename, temp_name);
    temp_name = strtok(NULL, "/");
  }
  int isRename = get_option_cmd(filename);
  printf("clientの設定したファイル名は%sです\n", filename);
  printf("%sに保存します\n", argv[3]);
  char save_path[512] = {'\0'};
  // 保存先に拡張子がある場合はファイル名まで指定していると判断
  // ファイル名と保存先パスの結合を行わない
  if (strstr(argv[3], ".") == NULL)
  {
    if (argv[3][strlen(argv[3]) - 1] != '/')
    {
      sprintf(save_path, "%s/%s", argv[3], filename);
    }
    else
    {
      sprintf(save_path, "%s%s", argv[3], filename);
    }
  }
#define __PATH_DEBAG 0
  if (__PATH_DEBAG)
  {
    printf("argv[3][strlen(argv[3])]: %c\nsave_path: %s\n", argv[3][strlen(argv[3]) - 1], save_path);
  }

  if ((fd = open(save_path, O_RDONLY)) >= 0)
  {
    close(fd);
    puts("同じ名前のファイルが存在しています\n上書きしますか？ Yes -> 1, No -> 0");
    if (safe_entry(1) == 0)
    {
      puts("終了します");
      close(sock);
      return 0;
    }
  }

  fd = open(save_path, O_WRONLY | O_CREAT, 0600);
  // fd = open(filename, O_WRONLY);
  if (fd < 0)
  {
    ERROR;
    perror("open");
    return 1;
  }
  // ファイルの送受信を始めさせるキーを送信
  if ((write(sock, "start", 5)) < 0)
  {
    ERROR;
    perror("write");
    close(sock);
    exit(1);
  }

  while ((n = read(sock, buf, sizeof(buf))) > 0)
  {
    // puts("[server]\nloop");
    ret = write(fd, buf, n);
    if (ret < 1)
    {
      ERROR;
      perror("write");
      break;
    }
  }

  /* socketの終了 */
  close(sock);

  return 0;
}
