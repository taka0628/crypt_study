#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/dh.h>

#define ERROR(comment)                                                \
    printf("[ERROR]\n\t%s: %d\n\t%s\n", __func__, __LINE__, comment); \
    exit(0);

typedef struct
{
    char *data;
    unsigned int size;
} dynamic_mem_t;

typedef struct
{
    dynamic_mem_t data;
    unsigned int str_len;
} chipher_data_t;

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

void decrypt(unsigned char *out, const unsigned char *in, const uint8_t *key, const uint8_t *iv, int size)
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

bool DH_code_check(int code)
{
    if (code & DH_CHECK_P_NOT_PRIME)
    {
        fprintf(stderr, "p value is not prime\n");
        return false;
    }
    if (code & DH_CHECK_P_NOT_SAFE_PRIME)
    {
        fprintf(stderr, "p value is not a safe prime\n");
        return false;
    }
    if (code & DH_UNABLE_TO_CHECK_GENERATOR)
    {
        fprintf(stderr, "unable to check the generator value\n");
        return false;
    }
    if (code & DH_NOT_SUITABLE_GENERATOR)
    {
        fprintf(stderr, "the g value is not a generator\n");
        return false;
    }
    return true;
}

bool DH_first_set(DH *a, int keyLen, int code)
{
    puts("DH start");
    DH_check(a, &code);
    if (DH_code_check(code) == false)
    {
        ERROR("DH_key is not safe");
        // puts("DH_key is not safe");
        exit(1);
    }
    printf("\n");
    DHparams_print_fp(stdout, a);
    printf("\n");
    return true;
}

void create_DH_key(DH *a, const BIGNUM *pub_key, const BIGNUM *priv_key)
{

    DH_generate_key(a);
    DH_get0_key(a, &pub_key, &priv_key);
    printf("pub_key: %s\npriv_key: %s\n", BN_bn2hex(pub_key), BN_bn2hex(priv_key));
}

// RSA暗号の鍵を生成
bool rsa_key_set(const unsigned int key_size, EVP_PKEY *pkey)
{
    printf("RSA-%dで鍵を生成します\n", key_size);
    int rc = 1;
    // RSAの設定
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx)
    {
        ERROR("EVP_PKEY_CTX");
        return false;
    }
    if (EVP_PKEY_keygen_init(ctx) <= 0)
    {
        ERROR("EVP_PKEY_keygen_init");
        return false;
    }
    if ((rc = EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, key_size)) <= 0)
    {
        ERROR("EVP_PKEY_CTX");
        return false;
    }
    //  鍵生成
    if ((rc = EVP_PKEY_keygen(ctx, &pkey)) <= 0)
    {
        ERROR("key_gen");
        return false;
    }
    EVP_PKEY_CTX_free(ctx);

    return true;
}

// メモリの動的確保と初期化、大きさの保存を行う
char *dynamic_new(dynamic_mem_t *data, size_t size)
{
    data->size = size;
    data->data = malloc(data->size);
    memset(data->data, 0, data->size);
    if (data->data == NULL)
    {
        printf("malloc size: %ld\n", size);
        ERROR("malloc");
    }
    return data->data;
}

// RSA暗号の公開鍵を取得
void get_rsa_key(EVP_PKEY *pkey, BIGNUM **pubn, BIGNUM **pube, BIGNUM **privk)
{
    RSA *rsa = RSA_new();
    rsa = EVP_PKEY_get1_RSA(pkey);
    if (pubn != NULL && pube != NULL)
    {
        RSA_get0_key(rsa, (const BIGNUM **)pubn, (const BIGNUM **)pube, NULL);
        printf("RSA KEY\npn: %s\npe %s\n", BN_bn2hex(*pubn), BN_bn2hex(*pube));
    }
    else if (privk != NULL)
    {
        RSA_get0_key(rsa, NULL, NULL, (const BIGNUM **)privk);
        printf("RSA KEY\nprivkey: %s\n", BN_bn2hex(*privk));
    }
    else
    {
        ERROR("不正な鍵の引数");
    }
}

void RSA_encrypt(EVP_PKEY *pkey, BIGNUM *pubn, BIGNUM *pube, dynamic_mem_t in, chipher_data_t *result)
{
    printf("平文: %s\n", in.data);

    RSA *rsa_pubkey = RSA_new();
    if (RSA_set0_key(rsa_pubkey, pubn, pube, NULL) <= 0)
    {
        ERROR("RSA_set0_key");
    }
    if (EVP_PKEY_set1_RSA(pkey, rsa_pubkey) <= 0)
    {
        ERROR("EVP_PKEY_set1_RSA");
    }

    dynamic_mem_t out;
    out.size = in.size;
    out.data = (char *)malloc(out.size);
    memset(out.data, 0, out.size);
    int outlen = RSA_public_encrypt(in.size, (const unsigned char *)in.data, (unsigned char *)out.data, rsa_pubkey, RSA_PKCS1_PADDING);
    if (outlen <= 0)
    {
        int error = ERR_get_error();
        char *str_error = malloc(256);
        str_error = (char *)ERR_reason_error_string(error);
        printf("%s\n", str_error);
        free(str_error);
        RSA_free(rsa_pubkey);
        ERROR("RSA_public_encrypt");
    }

    print("暗号化されたデータ: ", (uint8_t *)out.data, outlen);
    printf("outlen: %d\n", outlen);
    puts("data cpy");
    if (outlen <= result->data.size)
    {
        memcpy(result->data.data, out.data, outlen);
        result->str_len = outlen;
    }
    else
    {
        ERROR("lengs error");
    }
    RSA_free(rsa_pubkey);
    // EVP_PKEY_free(pkey);
}

void RSA_decrypt(BIGNUM *SKey, BIGNUM *pube, chipher_data_t in, chipher_data_t *out)
{
    print("暗号文: ", (uint8_t *)in.data.data, in.str_len);

    RSA *rsa_key = RSA_new();
    if (RSA_set0_key(rsa_key, NULL, pube, SKey) <= 0)
    {
        ERROR("RSA_set0_key");
    }
    chipher_data_t temp;
    temp.data.data = dynamic_new(&temp.data, in.str_len);
    temp.str_len = RSA_private_decrypt(in.str_len, (const unsigned char *)in.data.data, (unsigned char *)temp.data.data, rsa_key, RSA_PKCS1_PADDING);
    if (temp.str_len <= 0)
    {
        int error = ERR_get_error();
        char *str_error = malloc(256);
        str_error = (char *)ERR_reason_error_string(error);
        printf("%s\n", str_error);
        free(str_error);
        RSA_free(rsa_key);
        ERROR("RSA_private_decrypt");
    }

    printf("復号されたデータ: %s\n", temp.data.data);
    if (temp.str_len <= out->data.size)
    {
        memcpy(out->data.data, temp.data.data, temp.str_len);
        out->str_len = temp.str_len;
    }
    else
    {
        ERROR("lengs error");
    }
}
// bool get_DH_key(DH *dh, )

int main()
{
    time_t tm;
    time(&tm);
    unsigned char key[16] = {'\0'};
    unsigned char iv[16] = {'\0'};
    RAND_bytes(key, 128 / 8);
    print("key:\t", key, sizeof(key));
    RAND_bytes(iv, 128 / 8);
    print("iv:\t", iv, sizeof(iv));

    // RSA_generate_key_ex(rsa, 2048, bn, NULL);
    // // RSA_get0_key(rsa, NULL, &pubkey, &privkey);
    // EVP_PKEY_assign_RSA(p_key, rsa);
    // unsigned char str_pubkey[4096];
    // unsigned char str_privbkey[4096];
    // RSA_get0_key(rsa, &pubkey, )
    // // printf("pkey: %x, okey: %x\n", BN_bn2hex(p_key), BN_bn2hex(bn));
    // printf("pkey: %s, okey: %s\n", str_privbkey, str_pubkey);

    EVP_PKEY *pkey = EVP_PKEY_new();
    rsa_key_set(2048, pkey);

    BIGNUM *pubn = BN_new(), *pube = BN_new();
    get_rsa_key(pkey, &pubn, &pube, NULL);
    printf("鍵確認\npn: %s\npe: %s\n", BN_bn2hex(pubn), BN_bn2hex(pube));

    dynamic_mem_t in;
    in.data = dynamic_new(&in, sizeof("RSAテスト"));
    chipher_data_t out;
    out.data.data = dynamic_new(&out.data, 2048);
    strncpy(in.data, "RSAテスト", in.size);
    EVP_PKEY *inpkey = EVP_PKEY_new();
    RSA_encrypt(inpkey, pubn, pube, in, &out);
    print("main 受け取り: ", (uint8_t *)out.data.data, out.str_len);

    chipher_data_t de_text;
    de_text.data.data = dynamic_new(&de_text.data, out.data.size);
    de_text.str_len = 0;
    BIGNUM *SKey = BN_new();
    get_rsa_key(pkey, NULL, NULL, &SKey);
    RSA_decrypt(SKey, pube, out, &de_text);
    print("main 復号文受け取り: ", (uint8_t *)de_text.data.data, de_text.str_len);

    return 0;
}