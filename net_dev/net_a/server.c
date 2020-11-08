#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdbool.h>

#define PORT_NUM 4000
#define END_KEY "quit"

#define ERROR (printf("[ERROR]\n%s: %d\n", __func__, __LINE__))

void input_reception(char *result, int size)
{
    char *p = NULL;
    fgets(result, size, stdin);
    if ((p = strchr(result, '\n')) != NULL)
    {
        *p = '\0';
        p = NULL;
    }
}

bool string_check(char *sorce, char *key)
{
    if (strlen(sorce) == strlen(key) && strstr(sorce, key) != NULL)
    {
        return true;
    }
    return false;
}

int main()
{
    int sock0;
    struct sockaddr_in addr;
    struct sockaddr_in client;
    socklen_t len;
    int sock;

    /* ソケットの作成 */
    // SOCK_STREAM: 順序性と信頼性があり、双方向の、接続されたバイトストリーム (byte stream)を提供する。
    sock0 = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock0 < 0)
    {
        ERROR;
        perror("socket");
        printf("%d\n", errno);
        return 1;
    }

    /* ソケットの設定 */
    addr.sin_family = AF_INET;         //IPv4 インターネットプロトコル
    addr.sin_port = htons(PORT_NUM);   //ポート番号
    addr.sin_addr.s_addr = INADDR_ANY; // 利用可能なすべてのインターフェースにソケットをバインド
    // #define BIND_ERROR
    // アドレスを割り当て
    bool yes = 1;
    setsockopt(sock0, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
    int bind_error = bind(sock0, (struct sockaddr *)&addr, sizeof(addr));
    if (bind_error < 0)
    {
        ERROR;
        perror("bind");
        printf("%d\n", errno);
        close(sock);
        close(sock0);
        return 1;
    }

    /* TCPクライアントからの接続要求を待てる状態にする */
    // #define LISTEN_ERROR
    int listen_error = listen(sock0, 5);
    if (listen_error < 0)
    {
        ERROR;
        perror("listn");
        printf("%d\n", errno);
        close(sock);
        close(sock0);
        return 1;
    }

    /* TCPクライアントからの接続要求を受け付ける */
    len = sizeof(client);
    sock = accept(sock0, (struct sockaddr *)&client, &len);
    if (sock < 0)
    {
        ERROR;
        perror("accept");
        printf("%d\n", errno);
        close(sock);
        close(sock0);
        return 1;
    }

    char send_message[1024];
    char recieve_message[1024];
    char *temp;
    while (1)
    {
        memset(send_message, 0, sizeof(send_message));
        memset(recieve_message, 0, sizeof(recieve_message));
        if (read(sock, recieve_message, sizeof(recieve_message)) < 0)
        {
            ERROR;
            perror("read");
            close(sock);
            return 1;
        }
        printf("recieve<-%s\n", recieve_message);
        if (string_check(recieve_message, END_KEY))
        {
            puts("[end]");
            break;
        }
        printf("送信するメッセージを入力してください。\n>");
        input_reception(send_message, sizeof(send_message));
        printf("send->%s\n", send_message);
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

    sleep(1);
    /* TCPセッションの終了 */
    close(sock);

    /* listen するsocketの終了 */
    close(sock0);

    return 0;
}
