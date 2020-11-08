#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

int main()
{
    int sock0;
    struct sockaddr_in addr;
    struct sockaddr_in client;
    socklen_t len;
    int sock;

/* ソケットの作成 */
//   ここを変更
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
    int bind_error = bind(sock, (struct sockaddr *)&addr, 50);
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

/* 5文字送信 */
// #define WRITE_ERROR
#ifndef WRITE_ERROR
    int write_error = write(sock, "HELLO", 5);
#else
    int write_error = write(-1, "HELLO", 500);
#endif
    if (write_error < 0)
    {
        perror("write");
        printf("%d\n", errno);
        close(sock);
        close(sock0);
        return 1;
    }

    /* TCPセッションの終了 */
    close(sock);

    /* listen するsocketの終了 */
    close(sock0);

    return 0;
}
