#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 学生番号26001902600
#define ADDRESS ("239.126.100.1")
#define ERROR (printf("[ERROR]\n%s: %d\n", __func__, __LINE__))

// 送信側
int main()
{
    int sock_send;
    struct sockaddr_in addr;
    int n;
    sock_send = socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    /* 宛先マルチキャストアドレスの設定 */
    inet_pton(AF_INET, ADDRESS, &addr.sin_addr.s_addr);
    for (int i = 0; i < 10; i++)
    {
        n = sendto(sock_send, "HELLO", 5, 0, (struct sockaddr *)&addr, sizeof(addr));
        if (n < 1)
        {
            ERROR;
            perror("sendto");
            return 1;
        }
        sleep(1);
    }
    close(sock_send);
    return 0;
}
