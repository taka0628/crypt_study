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
#include <dirent.h>
#include <fcntl.h>

#include <openssl/evp.h>
#include <openssl/aes.h>

#define ERROR (printf("[ERROR]\n%s: %d\n", __func__, __LINE__))

#define PORT_NUM 12345
#define ERROR_KEY "error"
#define FILE_END_KEY "filecmp"
#define END_KEY "end"
#define BUF_SIZE 2048

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
    if (strchr(sorce, '\n') != NULL)
    {
        len++;
    }
    if (strlen(sorce) == len && strstr(sorce, key) != NULL)
    {
        return true;
    }
    return false;
}

bool send_file_check(char *filename)
{
    if (filename[0] != '.' && strchr(filename, '.'))
    {
        return true;
    }
    else if (strstr(filename, "makefile"))
    {
        return true;
    }
    return false;
}

void make_dir_path(char *dir, char *file_name, char *result)
{
    if (dir[strlen(dir)] == '/')
    {
        sprintf(result, "%s%s", dir, file_name);
    }
    else
    {
        sprintf(result, "%s/%s", dir, file_name);
    }
}

void encrypt(char *out, const char *in, const uint8_t *key, const uint8_t *iv, int size)
{
    int out_len, in_len = size;
    EVP_CIPHER_CTX *ctx;

    if (!(ctx = EVP_CIPHER_CTX_new()))
    {
        printf("error EVP_CIPHER_CTX_new\n");
        return;
    }

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_ctr(), NULL, key, iv))
    {
        printf("error EVP_EncryptInit_ex\n");
        return;
    }

    if (1 != EVP_EncryptUpdate(ctx, (unsigned char *)out, &out_len, (unsigned char *)in, in_len))
    {
        printf("error EVP_EncryptUpdate\n");
        return;
    }
    if (out_len != in_len)
    {
        printf("error encrypt out_len(%d) != in_len(%d)\n", out_len, in_len);
    }

    EVP_CIPHER_CTX_free(ctx);
}

void decrypt(char *out, const char *in, const uint8_t *key, const uint8_t *iv, int size)
{
    int out_len, in_len = size;
    EVP_CIPHER_CTX *ctx;

    if (!(ctx = EVP_CIPHER_CTX_new()))
    {
        printf("error EVP_CIPHER_CTX_new\n");
        return;
    }

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ctr(), NULL, key, iv))
    {
        printf("error EVP_DecryptInit_ex\n");
        return;
    }

    if (1 != EVP_DecryptUpdate(ctx, out, &out_len, in, in_len))
    {
        printf("error EVP_DecryptUpdate\n");
        return;
    }
    if (out_len != in_len)
    {
        printf("error decrypt out_len(%d) != in_len(%d)\n", out_len, in_len);
    }

    EVP_CIPHER_CTX_free(ctx);
}

void print(char *header, uint8_t *buf, int size)
{
    int i;
    printf("%s", header);
    for (i = 0; i < size; i++)
    {
        printf("%02x", buf[i]);
    }
    printf("\n");
}

#define CHIPHER_DUBAG 1

bool chipher_send(char *data, int size, int sock)
{
    static uint8_t key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    static uint8_t iv[16] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
    char buf[BUF_SIZE] = {'\0'};

    encrypt(buf, data, key, iv, size);
    if (CHIPHER_DUBAG && size < 100)
    {
        puts("\n[send]");
        printf("data_size: %d\n", size);
        printf("data: %s\n", data);
        print("enc: ", buf, size);
    }
    if (write(sock, buf, size) < 0)
    {
        ERROR;
        perror("write");
        return false;
    }
    return true;
}

bool chipher_recieve(char *data, int size, int sock)
{
    static uint8_t key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    static uint8_t iv[16] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
    char buf[BUF_SIZE] = {'\0'};

    int data_size = 0;
    if ((data_size = read(sock, buf, sizeof(buf))) < 0)
    {
        ERROR;
        perror("read");
        return false;
    }
    decrypt(data, buf, key, iv, data_size);
    if (CHIPHER_DUBAG && data_size < 100)
    {
        puts("\n[recieve]");
        printf("data_size: %d\n", data_size);
        print("data: ", buf, data_size);
        printf("dec: %s\n", data);
    }
    return true;
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server;
    int sock;
    char buf[BUF_SIZE];
    int n;
    char hostname[32];
    char adress[32];
    int port_num = PORT_NUM;

    puts("main st");
    if (argc == 4)
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
        puts("./hoge hostname sroce_path save_path");
        return (1);
    }
    puts("adress comp");

    DIR *sorce_dir = opendir(argv[2]);
    if (sorce_dir == NULL)
    {
        ERROR;
        return 1;
    }

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
    char send_data[1024];

    // 保存ディレクトリを送信
    chipher_send(argv[3], strlen(argv[3]), sock);

    memset(buf, 0, sizeof(buf));
    // ディレクトリ作成の結果を受信
    chipher_recieve(buf, sizeof(buf), sock);
    printf("受信: %s\n", buf);
    if (string_check(buf, ERROR_KEY))
    {
        puts("server側のエラーを受信したため終了します。");
        close(sock);
        return 1;
    }

    // ファイル送信
    struct dirent *dis_file;
    char file_path[1024] = {'\0'};
    int fd = 0;
    while ((dis_file = readdir(sorce_dir)) != NULL)
    {
        if (send_file_check(dis_file->d_name))
        {
            make_dir_path(argv[2], dis_file->d_name, file_path);
            // ファイル名送信
            printf("ファイル名: %s\n", dis_file->d_name);
            if (chipher_send(dis_file->d_name, strlen(dis_file->d_name), sock) == false)
            {
                close(sock);
                return 1;
            }
            memset(buf, 0, sizeof(buf));
            usleep(1000);
            // ファイルデータ送信
            if ((fd = open(file_path, O_RDONLY)) > 0)
            {
                int n = 0;
                // 送信
                while ((n = read(fd, buf, sizeof(buf))) > 0)
                {
                    if (chipher_send(buf, n, sock) == false)
                    {
                        close(sock);
                        return 1;
                    }
                    memset(buf, 0, sizeof(buf));
                    usleep(100);
                }
                usleep(1000);
                chipher_send(FILE_END_KEY, strlen(FILE_END_KEY), sock);
            }
            else
            {
                ERROR;
                puts("ファイルが存在しません");
            }
            usleep(100);
        }
    }
    usleep(1000);
    chipher_send(END_KEY, strlen(END_KEY), sock);

    /* socketの終了 */
    close(sock);
    return 0;
}
