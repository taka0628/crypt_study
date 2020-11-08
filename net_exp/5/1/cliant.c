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
#include <stdbool.h>

#define ERROR (printf("[ERROR]\n%s: %d\n", __func__, __LINE__))

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
        puts("./~ hostname portnum");
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
    int port_num = atoi(argv[2]);
    server.sin_port = htons(port_num);

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
        ERROR;
        perror("connect");
        printf("%d\n", errno);
        return 1;
    }

    // 送信
    char message[1024] = {'\0'};
    printf("送信するメッセージを入力してください\n>");
    fgets(message, sizeof(message), stdin);
    char *p = strchr(message, '\n');
    if (p != NULL)
    {
        *p = '\0';
    }
    // p = strstr(message, "end");
    // if (p != NULL && strlen(message) == 3)
    // {
    //     puts("end");
    //     isContine = false;
    // }
    printf("send->%s\n", message);
    if (write(sock, message, strlen(message)) < 0)
    {
        ERROR;
        perror("write: ");
        close(sock);
        exit(1);
    }

    // 受信
    if (read(sock, buf, sizeof(buf)) < 0)
    {
        ERROR;
        perror("read: ");
        close(sock);
        exit(1);
    }
    printf("recieve<-%s\n", buf);

    /* socketの終了 */
    close(sock);

    return 0;
}
