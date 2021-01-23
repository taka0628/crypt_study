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
#include <openssl/dh.h>
#include <openssl/engine.h>

#define ERROR(comment)                                                \
    printf("[ERROR]\n\t%s: %d\n\t%s\n", __func__, __LINE__, comment); \
    exit(0);

typedef struct
{
    char *data;
    unsigned int size;
} dynamic_mem_t;

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

// RSA暗号の公開鍵を取得
void get_rsa_pubkey(EVP_PKEY *pkey, BIGNUM **pubn, BIGNUM **pube)
{
    RSA *rsa = RSA_new();
    rsa = EVP_PKEY_get1_RSA(pkey);
    RSA_get0_key(rsa, (const BIGNUM **)pubn, (const BIGNUM **)pube, NULL);
    printf("RSA KEY\npn: %s\npe %s\n", BN_bn2hex(*pubn), BN_bn2hex(*pube));
}

void enchiper_RSA(EVP_PKEY *pkey, BIGNUM *pubn, BIGNUM *pube, dynamic_mem_t in, dynamic_mem_t *result)
{
    printf("平文: %s\n", in.data);

    RSA *rsa = RSA_new();
    RSA_set0_key(rsa, pubn, pube, NULL);
    EVP_PKEY_set1_RSA(pkey, rsa);
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx)
    {
        ERROR("EVP_PKEY_CTX_new");
    }
    if (EVP_PKEY_encrypt_init(ctx) <= 0)
    {
        ERROR("EVP_PKEY_encrypt_init");
    }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0)
    {
        ERROR("EVP_PKEY_CTX_set_rsa_padding");
    }
    int out_len = 0;
    if (EVP_PKEY_encrypt(ctx, NULL, (size_t *)&out_len, (unsigned char *)in.data, in.size) <= 0)
    {
        ERROR("first EVP_PKEY_encrypt");
    }
    printf("outlen: %d\n", out_len);
    unsigned char *out = malloc(out_len);
    if(!out){
        ERROR("out");
    }
    // if (EVP_PKEY_encrypt(ctx, out, &out_len, (const unsigned char *)in.data, in.size) <= 0)
    // {
    //     ERROR("EVP_PKEY_encrypt out");
    // }
    RSA_public_encrypt(RSA_size(rsa), (const unsigned char *)in.data, out, rsa, RSA_PKCS1_PADDING);
    print("暗号化されたデータ: ", out, out_len);
    result->size = (unsigned int)out_len;
    puts("malloc");
    printf("size: %d\n", result->size);
    puts("cpy");
    strncpy(result->data, out, result->size);
    RSA_free(rsa);
    EVP_PKEY_free(pkey);
    free(out);
    puts("return");
    return;
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
    get_rsa_pubkey(pkey, &pubn, &pube);
    printf("鍵確認\npn: %s\npe: %s\n", BN_bn2hex(pubn), BN_bn2hex(pube));

    dynamic_mem_t in;
    in.size = 1024;
    in.data = malloc(in.size);
    dynamic_mem_t out;
    out.data = (char *)malloc(1024);
    strncpy(in.data, "RSAテスト", in.size);
    EVP_PKEY *inpkey = EVP_PKEY_new();
    enchiper_RSA(inpkey, pubn, pube, in, &out);

    print("main 受け取り: ", out.data, out.size);

    return 0;
}