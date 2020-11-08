#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// 学生番号26001902600
#define ADDRESS ("239.126.100.1")
#define ERROR (printf("[ERROR]\n%s: %d\n", __func__, __LINE__))

// 受信側
int main()
{
    int sock_rcv;
    struct addrinfo hints, *res;
    int err, n;
    struct group_req greq;
    char buf[2048];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    err = getaddrinfo(NULL, "12345", &hints, &res);
    if (err != 0)
    {
        ERROR;
        printf("getaddrinfo : %s\n", gai_strerror(err));
        return 1;
    }

    sock_rcv = socket(res->ai_family, res->ai_socktype, 0);
    bind(sock_rcv, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    err = getaddrinfo(ADDRESS, NULL, &hints, &res);
    if (err != 0)
    {
        ERROR;
        printf("getaddrinfo : %s\n", gai_strerror(err));
        return 1;
    }

    memset(&greq, 0, sizeof(greq));
    greq.gr_interface = 0; /* 任意のネットワークインターフェースを利用 */
    /* getaddrinfo()の結果をgroup_req構造体へコピー */
    memcpy(&greq.gr_group, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    /* MCAST_JOIN_GROUPを利用してマルチキャストグループへJOIN */
    if (setsockopt(sock_rcv, IPPROTO_IP, MCAST_JOIN_GROUP, (char *)&greq, sizeof(greq)) != 0)
    {
        ERROR;
        perror("setsockopt");
        return 1;
    }
    // ファイルpathを取得
    char filename[1024] = {'\0'};
    n = recv(sock_rcv, filename, sizeof(filename), 0);
    printf("recieve<-filename: %s\n", filename);

    memset(buf, 0, sizeof(buf));

    // ファイル作成
    int fd = open(filename, O_WRONLY | O_CREAT, 0600);

    // ファイルデータ受信
    while ((n = read(sock_rcv, buf, sizeof(buf))) > 0)
    {
        // puts("[server]\nloop");
        int ret = write(fd, buf, n);
        if (ret < 1)
        {
            perror("write");
            break;
        }
        printf("n: %d\n", n);
        if (strcmp(buf, "end") == 0)
        {
            break;
        }
        memset(buf, 0, sizeof(buf));
    }
    puts("end");
    close(sock_rcv);
    return 0;
}
