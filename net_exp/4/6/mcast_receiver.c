#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

// 学生番号26001902600
#define ADDRESS ("239.126.100.1")
#define BUFF_SIZE (10000)   //受け取るパケット容量

#define DATA_ARRAY_SIZE (4) //スレッドへ渡す構造体の数。コンフリクトをさけるため複数個を設定

#define ERROR (printf("[ERROR]\n%s: %d\n", __func__, __LINE__))

typedef struct
{
    char file_data[BUFF_SIZE]; //ファイルデータ
    int fd;                    //書き込むファイルポインタ
    int data_size;             //ファイルのデータサイズ
} thread_contents_t;           //スレッドへ渡すデータの構造体

// ファイルへの書き込みを行うスレッド
void *thread_write(void *argument)
{
    thread_contents_t *data = argument;
    // printf("\tdata_size: %d\n", data->data_size);
    if (write(data->fd, data->file_data, data->data_size) < 0)
    {
        perror("write");
        exit(1);
    }
    memset(data->file_data, 0, sizeof(data->data_size));
    pthread_exit(NULL);
}

// 受信側
int main()
{
    int sock_rcv;
    struct addrinfo hints, *res;
    int err, n;
    struct group_req greq;

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

    // スレッドに渡すデータの用意
    pthread_t th;
    thread_contents_t *contents = malloc(sizeof(thread_contents_t) * DATA_ARRAY_SIZE);
    const int buff_size = sizeof(contents[0].file_data);
    for (int i = 0; i < 4; i++)
    {
        memset(contents[i].file_data, 0, buff_size);
        contents[i].fd = 0;
    }

    // ファイル名を取得
    char filename[1024] = {'\0'};
    if (recv(sock_rcv, filename, sizeof(filename), 0) < 0)
    {
        ERROR;
        perror("recv");
        close(sock_rcv);
        return 0;
    }
    printf("recieve<-filename: %s\n", filename);

    // ファイル作成
    contents[0].fd = open(filename, O_WRONLY | O_CREAT, 0600);
    for (int i = 0; i < DATA_ARRAY_SIZE - 1; i++)
    {
        contents[i + 1].fd = contents[i].fd;
    }

    int loop = 0;
    int idx = 0;
    // ファイルデータ受信
    while ((contents[idx].data_size = read(sock_rcv, contents[idx].file_data, buff_size)) > 0)
    {
        // printf("size: %d\n", contents[idx].data_size);
        if (pthread_create(&th, NULL, thread_write, &contents[idx]) != 0)
        {
            ERROR;
            perror("pthread_create");
            return 1;
        }
        // 次に使用する配列番号を計算
        idx++;
        if (idx >= DATA_ARRAY_SIZE)
        {
            idx = 0;
        }
        // printf("loop: %d\n", loop++);
    }
    puts("end");
    close(sock_rcv);
    for (int i = 0; i < DATA_ARRAY_SIZE; i++)
    {
        free(&contents[i]);
    }
    return 0;
}
