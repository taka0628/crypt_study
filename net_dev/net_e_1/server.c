#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdbool.h>
#include <ctype.h> //iscntrl
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

// 4543.3 req/s

#define PORT_NUM 8080
#define THREAD_SIZE 4
#define END_KEY "quit"

#define ERROR (printf("[ERROR]\n%s: %d\n", __func__, __LINE__))

// 動的確保するメモリの大きさも保存しておく構造体。
typedef struct
{
    char *array;
    int size;
} array_t;

typedef struct
{
    pthread_t th;       //スレッドID
    // pthread_mutex_t mp; //排他アクセス
    // pthread_cond_t sig; //シグナル
    int sock;           //通信するソケットID
    int th_idx;         //スレッド番号
    // bool is_wait;       //スレッドを待機するか？
    // bool is_use;        //スレッドが使用中か？
} thread_contents_t;

// メモリの動的確保をする際に確保したサイズも保存する
char *new_array(array_t *array, int const size)
{
    char *p = (char *)malloc(size);
    memset(p, 0, size);
    array->size = size;
    return p;
}

#define CHARACTER_DEBAG 0
// 制御文字も16進数表記で出力
void print_character_code(char *sorce, int size)
{
    if (CHARACTER_DEBAG)
    {
        for (int i = 0; i < strlen(sorce); i++)
        {
            if (iscntrl(sorce[i]) == 0)
            {
                printf("%c", sorce[i]);
            }
            else
            {
                printf("<0x%02x>", sorce[i]);
            }
        }
        puts("");
    }
}

#define PATH_DEBAG 0
// httpで送られてきた文字列から絶対パスを生成
void get_path_form_http(char *http, array_t *result_path)
{
    char temp_1[2048] = {'\0'};
    char temp_2[2048] = {'\0'};
    memcpy(temp_2, http, sizeof(temp_2));
    char *p = strtok(http, "\n");
    if (p == NULL)
    {
        ERROR;
        puts("パス内に改行文字が存在しません");
        return;
    }
    if (strstr(temp_2, "GET ") != NULL)
    {
        // printf("\npath: %s\n", http);
        sscanf(http, "GET %s HTTP%s", temp_2, temp_1);
        if (PATH_DEBAG)
            printf("\npath: %s\n", temp_2);
        sprintf(temp_1, "/home/taka/htdocs%s", temp_2);
        if (PATH_DEBAG)
            printf("\npath: %s\n", temp_1);
        strncpy(temp_2, temp_1, sizeof(temp_2));
        struct stat st;
        stat(temp_1, &st);
        if (PATH_DEBAG)
            printf("ISREG: %d\n", S_ISREG(st.st_mode));
        if (S_ISREG(st.st_mode))
        {
            if (PATH_DEBAG)
                puts("file");
            strncpy(result_path->array, temp_1, result_path->size);
            return;
        }
        else if (S_ISDIR(st.st_mode))
        {
            puts("dir");
            if (temp_1[strlen(temp_1) - 1] == '/')
            {
                sprintf(result_path->array, "%sindex.html", temp_2);
            }
            else
            {
                sprintf(result_path->array, "%s/index.html", temp_2);
            }
            printf("path: %s\n", result_path->array);
        }
    }
}

void end_connect(int sock, int sock0)
{
    puts("end");
    sleep(10);
    /* TCPセッションの終了 */
    close(sock);
    /* listen するsocketの終了 */
    close(sock0);
}

#define TH_DEBAG 1
#define HTTP_DEBAG 0
void *thread_process(void *argument)
{
    thread_contents_t *data = argument;
    if (TH_DEBAG)
        printf("thread[%d] crate\n", data->th_idx);

    int sock = 0;
    int sock0 = 0;
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    char recieve_message[1024];
    array_t path, send_data;
    int fd = 0;
    int n = 0;
    char exit_file_message[] = {"HTTP/1.0 200 OK\r\n"};
    char no_file_message[] = {"HTTP/1.0 404 Not Found\r\n"};

    while (1)
    {
        sock0 = data->sock;
        sock = accept(sock0, (struct sockaddr *)&client, &len);
        printf("thread[%i] active\n", data->th_idx);
        if (sock < 0)
        {
            ERROR;
            perror("accept");
            printf("%d\n", errno);
            close(sock);
            close(sock0);
            return NULL;
        }

        memset(recieve_message, 0, sizeof(recieve_message));
        if (read(sock, recieve_message, sizeof(recieve_message)) < 0)
        {
            ERROR;
            perror("read");
            close(sock);
            pthread_exit(NULL);
        }
        if (HTTP_DEBAG)
        {
            printf("%s\n", recieve_message);
            print_character_code(recieve_message, sizeof(recieve_message));
        }

        path.array = new_array(&path, 2048);
        send_data.array = new_array(&send_data, 2048);

        get_path_form_http(recieve_message, &path);

        if ((fd = open(path.array, O_RDONLY)) < 0)
        {
            puts("404");
            if (write(sock, no_file_message, strlen(no_file_message)) < 0)
            {
                ERROR;
                pthread_exit(NULL);
            }
            if (HTTP_DEBAG)
            {
                printf("send->");
                print_character_code(no_file_message, strlen(no_file_message));
            }
        }
        else
        {
            if (HTTP_DEBAG)
            {
                puts("exit file");
            }

            if (write(sock, exit_file_message, strlen(exit_file_message)) < 0)
            {
                ERROR;
                perror("write");
                close(sock);
                pthread_detach(pthread_self());
                pthread_exit(NULL);
            }
            write(sock, "\r\n", strlen("\r\n"));
            while ((n = read(fd, send_data.array, send_data.size)))
            {
                if (write(sock, send_data.array, n) < 0)
                {
                    close(sock);
                    pthread_detach(pthread_self());
                    pthread_exit(NULL);
                }
            }
            if (HTTP_DEBAG)
            {
                printf("all data send");
            }
        }
        free(send_data.array);
        free(path.array);
        close(sock);
        printf("\tthread[%i] sleep\n", data->th_idx);
    }

    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

int main()
{
    int sock0;
    struct sockaddr_in addr;
    int sock = 0;
    int port_num = PORT_NUM;

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
    addr.sin_port = htons(port_num);   //ポート番号
    addr.sin_addr.s_addr = INADDR_ANY; // 利用可能なすべてのインターフェースにソケットをバインド
    // #define BIND_ERROR
    // アドレスを割り当て
    bool yes = 1;
    setsockopt(sock0, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
    int bind_error = bind(sock0, (struct sockaddr *)&addr, sizeof(addr));
    if (bind_error < 0)
    {
        addr.sin_port = htons(++port_num);
        bind_error = bind(sock0, (struct sockaddr *)&addr, sizeof(addr));
        if (bind_error < 0)
        {
            ERROR;
            perror("bind");
            printf("%d\n", errno);
            close(sock);
            close(sock0);
            return 1;
        }
    }
    printf("port_num: %d\n", port_num);

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
    thread_contents_t *contents = (thread_contents_t *)malloc(sizeof(thread_contents_t) * THREAD_SIZE);
    for (int i = 0; i < THREAD_SIZE; i++)
    {
        // pthread_mutex_init(&contents[i].mp, NULL);
        // contents[i].is_wait = true; //trueにしている限りスレッドはスリープする
        // contents[i].is_use = false;
        contents[i].th_idx = i;
        contents[i].sock = sock0;
    }
    for (int i = 0; i < THREAD_SIZE; i++)
    {
        pthread_create(&contents[i].th, NULL, thread_process, &contents[i]);
    }

    while(1){
        sleep(1);
    }
    end_connect(sock, sock0);
    return 0;
}
