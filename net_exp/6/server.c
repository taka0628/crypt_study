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
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#include <openssl/evp.h>
#include <openssl/aes.h>

#define PORT_NUM 12345
#define ERROR_KEY "error"
#define FILE_END_KEY "filecmp"
#define END_KEY "end"
#define BUF_SIZE 2048

#define ERROR (printf("[ERROR]\n%s: %d\n", __func__, __LINE__))

void socket_close(int sock, int sock0)
{
	sleep(1);
	close(sock);
	close(sock0);
}

bool send_data(int sock, char *send_data, int size)
{
	if (write(sock, send_data, size) < 0)
	{
		ERROR;
		perror("write");
		return false;
	}
	return true;
}

bool receive_data(int sock, char *rcv_data, int size)
{
	if (read(sock, rcv_data, size) < 0)
	{
		ERROR;
		perror("read");
		return false;
	}
	return true;
}

// 文字列が一致するか？
bool string_check(char *source, char *key)
{
	int len = strlen(key);
	if (strchr(source, '\n') != NULL)
	{
		len++;
	}
	if (strlen(source) == len && strstr(source, key) != NULL)
	{
		return true;
	}
	return false;
}

void create_makefile(char *dir_path, DIR *dir)
{
	char file_path[1024] = {'\0'};
	char temp[1024] = {'\0'};
	struct dirent *dis_file;
	sprintf(file_path, "%s/makefile", dir_path);
	// sprintf(file_path, "%s/test_makefile", dir_path);
	int fd = open(file_path, O_WRONLY | O_CREAT, 0600);
	dir = opendir(dir_path);
	close(fd);

	FILE *fp;
	if ((fp = fopen(file_path, "w")) == NULL)
	{
		ERROR;
		return;
	}

	int source_cnt = 0;
	char source_name[10][1024] = {'\0'};
	// TARGET
	while ((dis_file = readdir(dir)) != NULL && source_cnt < 10)
	{
		if (strstr(dis_file->d_name, ".c") != NULL)
		{
			strncpy(source_name[source_cnt], dis_file->d_name, sizeof(source_name[0]));
			source_cnt++;
		}
	}

	char *p = NULL;
	for (int i = 0; i < source_cnt; i++)
	{
		memset(temp, 0, sizeof(temp));
		strncpy(temp, source_name[i], sizeof(temp));
		if ((p = strstr(temp, ".c")) != NULL)
		{
			*p = *(p + 1) = '\0';
			p = NULL;
		}
		fprintf(fp, "TARGET_%d := %s\n", i, temp);
	}
	for (int i = 0; i < source_cnt; i++)
	{
		fprintf(fp, "SOURCE_%d := %s\n", i, source_name[i]);
	}

	fprintf(fp, "CC := gcc\nCFLAGS :=\nRM := /bin/rm\n\nall:\n");
	for (int i = 0; i < source_cnt; i++)
	{
		fprintf(fp, "\t$(CC) $(CFLAGS) $(SOURCE_%d) -o $(TARGET_%d)\n", i, i);
	}
	fprintf(fp, "\n");
	for (int i = 0; i < source_cnt; i++)
	{
		fprintf(fp, "$(TARGET_%d): $(SOURCE_%d)\n\t$(CC) $(CFLAGS) $< -o $@\n\n", i, i);
	}
	fprintf(fp, "clean:\n");
	for (int i = 0; i < source_cnt; i++)
	{
		fprintf(fp, "\t$(RM) -rf $(TARGET_%d)\n", i);
	}

	fclose(fp);
	closedir(dir);
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

#define CHIPHER_DEBUG 1
bool chipher_send(char *data, int size, int sock)
{
	static uint8_t key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
	static uint8_t iv[16] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
	char buf[BUF_SIZE] = {'\0'};

	encrypt(buf, data, key, iv, size);
	if (CHIPHER_DEBUG && size < 100)
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

int chipher_receive(char *data, int size, int sock)
{
	static uint8_t key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
	static uint8_t iv[16] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
	char buf[BUF_SIZE] = {'\0'};

	int data_size = 0;
	if ((data_size = read(sock, buf, sizeof(buf))) < 0)
	{
		ERROR;
		perror("read");
		return -1;
	}
	decrypt(data, buf, key, iv, data_size);
	if (CHIPHER_DEBUG && data_size < 100)
	{
		puts("\n[receive]");
		printf("data_size: %d\n", data_size);
		print("data: ", buf, data_size);
		printf("dec: %s\n", data);
	}
	return data_size;
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
	addr.sin_family = AF_INET;		   //IPv4 インターネットプロトコル
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
		socket_close(sock, sock0);
		return 1;
	}

	/* TCPクライアントからの接続要求を待てる状態にする */
	// #define LISTEN_ERROR
	int listen_error = listen(sock0, 5);
	if (listen_error < 0)
	{
		ERROR;
		perror("listen");
		printf("%d\n", errno);
		socket_close(sock, sock0);
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
		socket_close(sock, sock0);
		return 1;
	}

	char buf[65536];
	char dir_path[1024];
	char *temp;
	memset(buf, 0, sizeof(buf));

	// 保存ディレクトリのパスを受信
	if (chipher_receive(buf, sizeof(buf), sock) < 0)
	{
		socket_close(sock, sock0);
		return 1;
	}
	printf("dir: %s\n", buf);

	// ディレクトリ作成
	if (mkdir(buf, 0755) == 0)
	{
		printf("ディレクトリを作成しました　%s\n", buf);
		// send_data(sock, "sucseeded", 9);
		chipher_send("succeeded", 9, sock);
		memcpy(dir_path, buf, sizeof(dir_path));
		memset(buf, 0, sizeof(buf));
	}
	else
	{
		puts("ディレクトリの作成に失敗しました");
		// if (send_data(sock, ERROR_KEY, 3) == false)
		// {
		//   socket_close(sock, sock0);
		//   return 1;
		// }
		chipher_send(ERROR_KEY, 3, sock);
	}

	int fd = 0;
	char file_path[1024] = {'\0'};
	while (1)
	{
		memset(file_path, 0, sizeof(file_path));
		memset(buf, 0, sizeof(buf));
		// ファイル名受信
		if (chipher_receive(buf, sizeof(buf), sock) < 0)
		{
			socket_close(sock, sock0);
			return 1;
		}
		if (string_check(buf, END_KEY))
		{
			puts("受信を終了します");
			break;
		}
		if (string_check(buf, FILE_END_KEY) == false)
		{
			printf("受信ファイル名: %s\n", buf);
			sprintf(file_path, "%s/%s", dir_path, buf);
			printf("ファイル名: %s\n", file_path);
			fd = open(file_path, O_WRONLY | O_CREAT, 0600);
			if (fd < 0)
			{
				ERROR;
				perror("open");
				return 1;
			}
		}

		memset(buf, 0, sizeof(buf));
		int n = 0;
		while ((n = chipher_receive(buf, sizeof(buf), sock)) > 0)
		{
			if (string_check(buf, FILE_END_KEY) == false)
			{
				// puts("[server]\nloop");
				if (write(fd, buf, n) < 0)
				{
					ERROR;
					perror("write");
					socket_close(sock, sock0);
					return 1;
				}
				memset(buf, 0, sizeof(buf));
			}
			else
			{
				close(fd);
				break;
			}
		}
	}

	struct dirent *dis_file;
	bool isMakefileExit = false;
	DIR *dir = opendir(dir_path);
	while ((dis_file = readdir(dir)) != NULL)
	{
		if (string_check(dis_file->d_name, "makefile"))
		// if (string_check(dis_file->d_name, "test_makefile"))
		{
			isMakefileExit = true;
		}
	}
	if (isMakefileExit == false)
	{
		create_makefile(dir_path, dir);
	}

	socket_close(sock, sock0);
	return 0;
}
