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
#include <stdbool.h>

#define ERROR (printf("[ERROR]\n%s: %d\n", __func__, __LINE__))

#define PORT_NUM 8080
#define END_KEY "quit"

#define __ADRESS
#ifdef __ADRESS
void get_adress(char *hostname)
{
    struct addrinfo hints, *res;
    struct in_addr addr;
    char buf[16];
    int err;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    //  addrinfo 構造体のメモリー確保
    if ((err = getaddrinfo(hostname, NULL, &hints, &res)) != 0)
    {
        ERROR;
        printf("error %d\n", err);
        exit(1);
    }

    addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
    // アドレスをバイナリ形式からテキスト形式に変換する
    inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    printf("ip address : %s\n", buf);
    // メモリ解放
    freeaddrinfo(res);
}
#endif

// 改行文字を削除する標準入力
void input_reception(char *result, int size)
{
    char *p = NULL;
    fgets(result, size, stdin);
    if ((p = strchr(result, '\n')) != NULL)
    {
        // *p = '\0';
        p = NULL;
    }
}

// 文字列が一致するか？
bool string_check(char *sorce, char *key)
{
    int len = strlen(key);
    if(strchr(sorce, '\n') != NULL){
        len++;
    }
    if (strlen(sorce) == len && strstr(sorce, key) != NULL)
    {
        return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server;
    int sock;
    char buf[32];
    int n;
    char hostname[32];
    char adress[32];
    int port_num = PORT_NUM;

    puts("main st");
    if (argc == 2)
    {
        puts("argc == 3 st");
        strcpy(hostname, argv[1]);
        printf("hostname: %s\n", hostname);
        printf("port_num: %d\n", port_num);
        struct addrinfo hints, *res;
        struct in_addr addr;
        char buf[16];
        int err;

        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_family = AF_INET;
        if ((err = getaddrinfo(hostname, NULL, &hints, &res)) != 0)
        {
            ERROR;
            printf("error %d\n", err);
            return 1;
        }

        addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
        inet_ntop(AF_INET, &addr, buf, sizeof(buf));
        printf("ip address : %s\n", buf);

        freeaddrinfo(res);
        strcpy(adress, buf);
        printf("adress: %s\n", adress);
    }
    else
    {
        puts("./hoge hostname");
        return (1);
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
    int connect_error = connect(sock, (struct sockaddr *)&server, sizeof(server));
    if (connect_error < 0)
    {
        ERROR;
        perror("connect");
        printf("%d\n", errno);
        return 1;
    }
    char send_message[1024];
    char *temp;
    while (1)
    {
        memset(send_message, 0, sizeof(send_message));

        printf("送信するメッセージを入力してください。\n>");
        input_reception(send_message, sizeof(send_message));
        printf("send->%s\n", send_message);
        // 送信
        if (write(sock, send_message, strlen(send_message)) < 0)
        {
            ERROR;
            perror("write");
            close(sock);
            return 1;
        }
        if (string_check(send_message, END_KEY))
        {
            puts("[end]");
            break;
        }
        puts("");
    }

    /* socketの終了 */
    close(sock);

    return 0;
}
