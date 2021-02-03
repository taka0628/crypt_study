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

RSA *create_RSA(RSA *keypair, int pem_type, char *file_name)
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
        PEM_write_RSAPrivateKey(fp, keypair, NULL, NULL, NULL, NULL, NULL);
        fclose(fp);

        fp = fopen(file_name, "rb");
        PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL);
        fclose(fp);
    }

    return rsa;
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
}

// RSA暗号の鍵を生成
bool rsa_key_create_evp(const unsigned int key_size, RSA *priv_key, RSA *pub_key)
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
    EVP_PKEY *pkey = EVP_PKEY_new();
    if ((rc = EVP_PKEY_keygen(ctx, &pkey)) <= 0)
    {
        ERROR("key_gen");
        return false;
    }
    EVP_PKEY_CTX_free(ctx);

    RSA *keypair = RSA_new();
    keypair = EVP_PKEY_get0_RSA(pkey);
    EVP_PKEY_free(pkey);

    priv_key = create_RSA(keypair, PRIVATE_KEY_PEM, PRIVATE_KEY_FILE);
    pub_key = create_RSA(keypair, PUBLIC_KEY_PEM, PUBLIC_KEY_FILE);
    RSA_free(keypair);

    return true;
}

// メモリの動的確保と初期化、大きさの保存を行う
char *dynamic_new(dynamic_mem_t *data, size_t size)
{
    data->size = size;
    // data->data = malloc(data->size);
    data->data = calloc(data->size, sizeof(char));
    if (data->data == NULL)
    {
        printf("malloc size: %ld\n", size);
        ERROR("malloc");
    }
    return data->data;
}

char *dynamic_free(dynamic_mem_t *data)
{
    data->size = 0;
    free(data->data);
    data->data = NULL;
    return data->data;
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

#ifdef get_rsa_key
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
    RSA_free(rsa);
}
#endif

void RSA_encrypt_evp(BIGNUM *pubn, BIGNUM *pube, dynamic_mem_t in, chipher_data_t *result)
{
    printf("平文: %s\n", in.data);

    // 鍵をpkeyへ設定
    EVP_PKEY_CTX *ctx;
    EVP_PKEY *pkey = EVP_PKEY_new();
    RSA *rsa_pubk = RSA_new();
    if (RSA_set0_key(rsa_pubk, pubn, pube, NULL) <= 0)
    {
        EVP_PKEY_free(pkey);
        RSA_free(rsa_pubk);
        ERROR("RSA_set0_key");
    }
    if (EVP_PKEY_set1_RSA(pkey, rsa_pubk) <= 0)
    {
        EVP_PKEY_free(pkey);
        RSA_free(rsa_pubk);
        ERROR("EVP_PKEY_set1_RSA");
    }
    RSA_free(rsa_pubk);

    // アルゴリズムの設定
    ctx = EVP_PKEY_CTX_new(pkey, NULL);
    EVP_PKEY_free(pkey);
    if (!ctx)
    {
        ERROR("EVP_PKEY_CTX_new");
    }
    if (EVP_PKEY_encrypt_init(ctx) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        ERROR("EVP_PKEY_encrypt_init");
    }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        ERROR("EVP_PKEY_CTX_set_rsa_padding");
    }

    // 暗号化に必要な出力サイズを取得
    size_t outlen = 0;
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, in.data, in.size) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        ERROR("EVP_PKEY_encrypt, get outlen");
    }
    unsigned char *out = OPENSSL_malloc(outlen);
    if (!out)
    {
        EVP_PKEY_CTX_free(ctx);
        ERROR("OPENSSL_malloc");
    }

    // 暗号化
    if (EVP_PKEY_encrypt(ctx, out, &outlen, in.data, in.size) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_free("out");
        ERROR("EVP_PKEY_encrypt");
    }
    print("暗号化されたデータ: ", out, outlen);
    if (result->data.size >= outlen)
    {
        memcpy(result->data.data, out, outlen);
        result->str_len = outlen;
    }
    else
    {
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_free(out);
        ERROR("memcpy");
    }

    EVP_PKEY_CTX_free(ctx);
    OPENSSL_free(out);
}

void RSA_encrypt(RSA *pub_key, dynamic_mem_t in, chipher_data_t *result)
{
    printf("平文: %s\n", in.data);
    dynamic_mem_t out;
    out.data = dynamic_new(&out, RSA_size(pub_key));

    // 暗号化
    int outlen = RSA_public_encrypt(strlen(in.data) + 1, (unsigned char *)in.data, (unsigned char *)out.data, pub_key, RSA_PKCS1_OAEP_PADDING);
    if (outlen <= 0)
    {
        int error = ERR_get_error();
        char *str_error = malloc(256);
        str_error = (char *)ERR_reason_error_string(error);
        printf("%s\n", str_error);
        free(str_error);
        RSA_free(pub_key);
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
    // EVP_PKEY_free(pkey);
}

void RSA_decrypt_evp(EVP_PKEY *pkey, chipher_data_t in, chipher_data_t *out)
{
    print("暗号文: ", (uint8_t *)in.data.data, in.str_len);

    // 鍵をpkeyへ設定
    RSA *rsa_priv = get_rsa_key(PRIVATE_KEY_PEM, PRIVATE_KEY_FILE);
    EVP_PKEY *priv_key = EVP_PKEY_new();
    if (EVP_PKEY_set1_RSA(priv_key, rsa_priv) <= 0)
    {
        EVP_PKEY_free(priv_key);
        RSA_free(rsa_priv);
        ERROR("EVP_PKEY_set1_RSA");
    }
    RSA_free(rsa_priv);

    // アルゴリズムの設定
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(priv_key, NULL);
    EVP_PKEY_free(priv_key);
    if (!ctx)
    {
        ERROR("EVP_PKEY_CTX_new");
    }
    if (EVP_PKEY_decrypt_init(ctx) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        ERROR("EVP_PKEY_decrypt_init");
    }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        ERROR("EVP_PKEY_CTX_set_rsa_padding");
    }

    // 復号に必要な出力サイズを取得
    size_t outlen = 0;
    int error = 0;
    if ((error = EVP_PKEY_decrypt(ctx, NULL, &outlen, (const unsigned char *)in.data.data, (size_t)in.str_len)) <= 0)
    {
        if (error == -2)
        {
            EVP_PKEY_CTX_free(ctx);
            ERROR("操作が公開鍵アルゴリズムでサポートされていません");
        }
        EVP_PKEY_CTX_free(ctx);
        ERROR("EVP_PKEY_decrypt, get outlen");
    }
    unsigned char *out_buf = OPENSSL_malloc(outlen);
    if (!out_buf)
    {
        EVP_PKEY_CTX_free(ctx);
        ERROR("OPENSSL_malloc");
    }

    // 復号
    if ((error = EVP_PKEY_decrypt(ctx, out_buf, &outlen, in.data.data, in.str_len)) <= 0)
    {
        if (error == -2)
        {
            EVP_PKEY_CTX_free(ctx);
            OPENSSL_free("out");
            ERROR("操作が公開鍵アルゴリズムでサポートされていません");
        }
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_free("out");
        ERROR("EVP_PKEY_decrypt");
    }
    print("暗号化されたデータ: ", out, outlen);
    if (out->data.size >= outlen)
    {
        memcpy(out->data.data, out_buf, outlen);
        out->str_len = outlen;
    }
    else
    {
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_free(out);
        ERROR("memcpy");
    }

    EVP_PKEY_CTX_free(ctx);
    OPENSSL_free(out);
}

void RSA_decrypt(EVP_PKEY *pkey, chipher_data_t in, chipher_data_t *out)
{
    print("暗号文: ", (uint8_t *)in.data.data, in.str_len);

    // 鍵設定
    RSA *priv_key = RSA_new();
    priv_key = EVP_PKEY_get0_RSA(pkey);
    BIGNUM *priv_temp = BN_new();
    RSA_get0_key(priv_key, NULL, NULL, priv_temp);
    RSA_free(priv_key);
    priv_key = RSA_new();
    RSA_set0_key(priv_key, NULL, NULL, priv_temp);
    BN_free(priv_temp);

    chipher_data_t temp;
    temp.data.data = dynamic_new(&temp.data, in.str_len);
    temp.str_len = RSA_private_decrypt(in.str_len, (const unsigned char *)in.data.data, (unsigned char *)temp.data.data, priv_key, RSA_PKCS1_PADDING);
    if (temp.str_len <= 0)
    {
        int error = ERR_get_error();
        char *str_error = malloc(256);
        str_error = (char *)ERR_reason_error_string(error);
        printf("%s\n", str_error);
        free(str_error);
        RSA_free(priv_key);
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
        RSA_free(priv_key);
        ERROR("lengs error");
    }
    RSA_free(priv_key);
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

    // 鍵生成
    rsa_key_create(2048);
    RSA *pub_key = get_rsa_key(PUBLIC_KEY_PEM, PUBLIC_KEY_FILE);
    RSA *priv_key = get_rsa_key(PRIVATE_KEY_PEM, PRIVATE_KEY_FILE);

    // 暗号化
    dynamic_mem_t in;
    in.data = dynamic_new(&in, sizeof("RSAテスト"));
    chipher_data_t out;
    out.data.data = dynamic_new(&out.data, 2048);
    out.str_len = 0;
    strncpy(in.data, "RSAテスト", in.size);
    RSA_encrypt(pub_key, in, &out);
    // RSA_encrypt_evp(pubn, pube, in, &out);
    print("main 受け取り: ", (uint8_t *)out.data.data, out.str_len);
#ifdef temp
    // 復号
    printf("\n[復号]\n");
    chipher_data_t dec_text; //復号文
    chipher_data_t enc_text; //暗号文
    enc_text.data.data = dynamic_new(&enc_text.data, out.str_len);
    enc_text.str_len = out.str_len;
    memcpy(enc_text.data.data, out.data.data, out.str_len);
    dec_text.data.data = dynamic_new(&dec_text.data, 5048);
    dec_text.str_len = 0;
    // BIGNUM *SKey = BN_new();
    // get_rsa_key(pkey, NULL, NULL, &SKey);
    // RSA_decrypt(pkey, enc_text, &dec_text);
    RSA_decrypt_evp(pkey, enc_text, &dec_text);
    print("main 復号文受け取り: ", (uint8_t *)dec_text.data.data, dec_text.str_len);

    in.data = dynamic_free(&in);
    out.data.data = dynamic_free(&out.data);
    dec_text.data.data = dynamic_free(&dec_text.data);
    enc_text.data.data = dynamic_free(&enc_text.data);

#endif
    return 0;
}