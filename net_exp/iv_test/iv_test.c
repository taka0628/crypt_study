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
#include <openssl/dh.h>

#define ERROR (printf("[ERROR]\n%s: %d\n", __func__, __LINE__))

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
        ERROR;
        puts("DH_key is not safe");
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

    DH *a = DH_new();
    const BIGNUM *a_pub_key = NULL;
    const BIGNUM *a_priv_key = NULL;
    int keyLen = 64;
    int code = 0;
    DH_generate_parameters_ex(a, keyLen, DH_GENERATOR_5, NULL);
    DH_first_set(a, keyLen, code);

    puts("A");
    create_DH_key(a, a_pub_key, a_priv_key);

    puts("B");
    DH *b = DH_new();
    DH_generate_parameters_ex(b, keyLen, DH_GENERATOR_5, NULL);
    DH_first_set(b, keyLen, code);
    const BIGNUM *b_pub_key = NULL;
    const BIGNUM *b_priv_key = NULL;
    create_DH_key(b, b_pub_key, b_priv_key);

    return 0;
}