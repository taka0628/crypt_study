#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 学生番号26001902600
#define ADDRESS ("239.126.100.1")
#define ERROR (printf("[ERROR]\n%s: %d\n\n", __func__, __LINE__))

// 送信側
int main(int argc, char *argv[])
{
    int fd;
    if (argc != 2)
    {
        ERROR;
        puts("./hoge filename");
        _exit(1);
    }
    else
    {
        if ((fd = open(argv[1], O_RDONLY)) < 0)
        {
            ERROR;
            puts("同じ名前のファイルは存在していません");
            exit(1);
        }
        else
        {
            printf("argv: %s\n", argv[1]);
            printf("fd: %d\n", fd);
        }
    }
    int sock_send;
    struct sockaddr_in addr;
    int n;
    sock_send = socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    /* 宛先マルチキャストアドレスの設定 */
    inet_pton(AF_INET, ADDRESS, &addr.sin_addr.s_addr);
    char buf[65536] = {'\0'};
    printf("sizeof(buf): %ld\n", sizeof(buf));

    // pathからファイル名の取得
    char *temp_name;
    char filename[1024] = {'\0'};
    temp_name = strtok(argv[1], "/");
    while (temp_name != NULL)
    {
        strcpy(filename, temp_name);
        temp_name = strtok(NULL, "/");
    }

    int ret = sendto(sock_send, filename, sizeof(filename), 0, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 1)
    {
        ERROR;
        perror("sendto");
        close(sock_send);
        exit(1);
    }
    for (int i = 0; i < 10; i++)
    {
        while ((n = read(fd, buf, sizeof(buf))) > 0) //ファイル読み込み
        {
            // printf("n: %d\n", n);
            ret = sendto(sock_send, buf, n, 0, (struct sockaddr *)&addr, sizeof(addr));
            if (ret < 1)
            {
                ERROR;
                perror("sendto");
                break;
            }
        }
    }

    ret = sendto(sock_send, "end", 3, 0, (struct sockaddr *)&addr, sizeof(addr));
    close(sock_send);
    return 0;
}
