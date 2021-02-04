#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/dh.h>

#define ERROR(comment)                                                \
    printf("[ERROR]\n\t%s: %d\n\t%s\n", __func__, __LINE__, comment); \
    exit(0);

#define PUBLIC_KEY_PEM 1
#define PRIVATE_KEY_PEM 0
#define PRIVATE_KEY_FILE "private_key"
#define PUBLIC_KEY_FILE "public_key"
#define KEY_SIZE 2048

typedef struct
{
    char *data;    //データ
    size_t size;   //メモリサイズ
    size_t strlen; //文字数
} dynamic_mem_t;

void print(char *header, uint8_t *buf, int size)
{
    int i;
    printf("%s", header);
    for (i = 0; i < size; i++)
    {
        printf("%02x", buf[i]);
    }
    sync();
    printf("\n");
}

void create_RSA(RSA *keypair, int pem_type, char *file_name)
{

    RSA *rsa = NULL;
    FILE *fp = NULL;

    if (pem_type == PUBLIC_KEY_PEM)
    {

        fp = fopen(file_name, "w");
        PEM_write_RSAPublicKey(fp, keypair);
        fclose(fp);

        fp = fopen(file_name, "rb");
        PEM_read_RSAPublicKey(fp, &rsa, NULL, NULL);
        fclose(fp);
    }
    else if (pem_type == PRIVATE_KEY_PEM)
    {

        fp = fopen(file_name, "w");
        PEM_write_RSAPrivateKey(fp, keypair, NULL, NULL, KEY_SIZE, NULL, NULL);
        fclose(fp);

        fp = fopen(file_name, "rb");
        PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL);
        fclose(fp);
    }
    RSA_free(rsa);
}

void rsa_key_create(const size_t key_len)
{
    BIGNUM *bn;
    bn = BN_new();
    BN_set_word(bn, RSA_F4);

    RSA *keypair;
    keypair = RSA_new();
    RSA_generate_key_ex(keypair, key_len, bn, NULL);
    create_RSA(keypair, PRIVATE_KEY_PEM, PRIVATE_KEY_FILE);
    create_RSA(keypair, PUBLIC_KEY_PEM, PUBLIC_KEY_FILE);
    BN_free(bn);
    RSA_free(keypair);
}

// メモリの動的確保と初期化、大きさの保存を行う
char *dynamic_new(dynamic_mem_t *data, size_t size)
{
    data->data = calloc(size, sizeof(char));
    data->size = size;
    data->strlen = 0;
    if (data->data == NULL)
    {
        printf("malloc size: %ld\n", size);
        ERROR("malloc");
    }
    return data->data;
}

void dynamic_cpy(dynamic_mem_t *to, dynamic_mem_t *from)
{
    bool canCpy = false;
    if (to->data != NULL && from->data != NULL && from->strlen > 0)
    {
        if (to->size >= from->strlen)
        {
            canCpy = true;
        }
    }
    if (canCpy)
    {
        memcpy(to->data, from->data, from->strlen);
        to->strlen = from->strlen;
    }
    else
    {
        printf("[from]\n\t data: %s\n", from->data);
        printf("\tsize: %ld, len: %ld\n", from->size, from->strlen);
        printf("[to]\n\t data: %s\n", to->data);
        printf("\tsize: %ld, len: %ld\n", to->size, to->strlen);
        ERROR("dynamic_cpy");
    }
}

void dynamic_free(dynamic_mem_t *data)
{
    data->size = 0;
    data->strlen = 0;
    free(data->data);
    data->data = NULL;
}

RSA *get_rsa_key(int mode, char *file_name)
{
    RSA *rsa = RSA_new();
    FILE *fp = NULL;
    if (mode == PUBLIC_KEY_PEM)
    {
        fp = fopen(file_name, "rb");
        PEM_read_RSAPublicKey(fp, &rsa, NULL, NULL);
        fclose(fp);
    }
    else if (mode == PRIVATE_KEY_PEM)
    {
        fp = fopen(file_name, "rb");
        PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL);
        fclose(fp);
    }
    else
    {
        ERROR("引数の設定に誤りがあります");
    }
    return rsa;
}

void RSA_encrypt(RSA *pub_key, dynamic_mem_t in, dynamic_mem_t *result)
{
    // printf("平文: %s\n", in.data);
    print("平文: ", (uint8_t *)in.data, in.strlen);
    dynamic_mem_t out;
    out.data = dynamic_new(&out, RSA_size(pub_key));

    // 暗号化
    out.strlen = RSA_public_encrypt(strlen(in.data), (unsigned char *)in.data, (unsigned char *)out.data, pub_key, RSA_PKCS1_OAEP_PADDING);
    if (out.strlen <= 0)
    {
        int error = ERR_get_error();
        char *str_error = malloc(256);
        str_error = (char *)ERR_reason_error_string(error);
        printf("%s\n", str_error);
        free(str_error);
        RSA_free(pub_key);
        ERROR("RSA_public_encrypt");
    }

    print("暗号化されたデータ: ", (uint8_t *)out.data, out.strlen);
    printf("outlen: %ld\n", out.strlen);
    puts("data cpy");
    dynamic_cpy(result, &out);
    dynamic_free(&out);
    // EVP_PKEY_free(pkey);
}

void RSA_decrypt(RSA *priv_key, dynamic_mem_t in, dynamic_mem_t *out)
{
    print("暗号文: ", (uint8_t *)in.data, in.strlen);

    // 暗号化
    dynamic_mem_t temp;
    temp.data = dynamic_new(&temp, in.strlen);
    temp.strlen = RSA_private_decrypt(in.strlen, (const unsigned char *)in.data, (unsigned char *)temp.data, priv_key, RSA_PKCS1_OAEP_PADDING);
    if (temp.strlen <= 0)
    {
        int error = ERR_get_error();
        char *str_error = malloc(256);
        str_error = (char *)ERR_reason_error_string(error);
        printf("%s\n", str_error);
        free(str_error);
        RSA_free(priv_key);
        ERROR("RSA_private_decrypt");
    }

    // printf("復号されたデータ: %s\n", temp.data);
    print("復号されたデータ: ", (uint8_t *)temp.data, temp.strlen);
    if (temp.strlen <= out->size)
    {
        dynamic_cpy(out, &temp);
    }
    else
    {
        RSA_free(priv_key);
        ERROR("lengs error");
    }
    dynamic_free(&temp);
    temp.strlen = 0;
}
// bool get_DH_key(DH *dh, )

int main()
{
    // 鍵生成
    rsa_key_create(KEY_SIZE);
    RSA *pub_key = get_rsa_key(PUBLIC_KEY_PEM, PUBLIC_KEY_FILE);
    RSA *priv_key = get_rsa_key(PRIVATE_KEY_PEM, PRIVATE_KEY_FILE);

    printf("key_size\n pub_key: %d\npriv_key: %d\n", RSA_size(pub_key), RSA_size(priv_key));

    int rand_size = 256 / 8;
    unsigned char *rand_buf = (unsigned char *)calloc(1024, sizeof(unsigned char));
    if (RAND_bytes((unsigned char *)rand_buf, rand_size) <= 0)
    {
        int error = ERR_get_error();
        char *str_error = malloc(256);
        str_error = (char *)ERR_reason_error_string(error);
        printf("%s\n", str_error);
        free(str_error);
        ERROR("RAND_bytes");
    }
    print("RAND: ", (uint8_t *)rand_buf, rand_size);

    // 暗号化
    dynamic_mem_t in;
    in.data = dynamic_new(&in, KEY_SIZE + 1);
    dynamic_mem_t out;
    out.data = dynamic_new(&out, KEY_SIZE);
    memcpy(in.data, rand_buf, rand_size);
    in.strlen = rand_size;
    // strncpy(in.data, shared_key.data, in.size);
    RSA_encrypt(pub_key, in, &out);
    // RSA_encrypt_evp(pubn, pube, in, &out);
    print("main 受け取り: ", (uint8_t *)out.data, out.strlen);

    // 復号
    printf("\n[復号]\n");
    dynamic_mem_t dec_text; //復号文
    dynamic_mem_t enc_text; //暗号文
    enc_text.data = dynamic_new(&enc_text, out.strlen);
    dynamic_cpy(&enc_text, &out);
    dec_text.data = dynamic_new(&dec_text, KEY_SIZE);
    RSA_decrypt(priv_key, enc_text, &dec_text);
    // printf("main 復号文受け取り: %s\n", dec_text.data);
    print("main 復号文受け取り: ", (uint8_t *)dec_text.data, dec_text.strlen);

    dynamic_free(&in);
    dynamic_free(&out);
    dynamic_free(&dec_text);
    dynamic_free(&enc_text);

    RSA_free(priv_key);
    RSA_free(pub_key);

    return 0;
}