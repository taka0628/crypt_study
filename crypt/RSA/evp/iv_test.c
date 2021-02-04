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
EVP_PKEY *rsa_key_create_evp(const unsigned int key_size)
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

    return pkey;
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

void RSA_encrypt_evp(BIGNUM *pubn, BIGNUM *pube, dynamic_mem_t in, dynamic_mem_t *result)
{
    // printf("平文: %s\n", in.data);

    // 鍵をpkeyへ設定
    EVP_PKEY *pkey = EVP_PKEY_new();
    RSA *rsa_pubkey = RSA_new();
    if (RSA_set0_key(rsa_pubkey, pubn, pube, NULL) <= 0)
    {
        EVP_PKEY_free(pkey);
        RSA_free(rsa_pubkey);
        ERROR("RSA_set0_key");
    }
    if (EVP_PKEY_set1_RSA(pkey, rsa_pubkey) <= 0)
    {
        EVP_PKEY_free(pkey);
        RSA_free(rsa_pubkey);
        ERROR("EVP_PKEY_set1_RSA");
    }
    printf("RSA key size: %d\n", RSA_size(rsa_pubkey));

    // アルゴリズムの設定
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL);
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
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        ERROR("EVP_PKEY_CTX_set_rsa_padding");
    }

    // 暗号化に必要な出力サイズを取得
    size_t outlen = 0;
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, in.data, RSA_size(rsa_pubkey)) <= 0)
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
    RSA_free(rsa_pubkey);

    // 暗号化
    if (EVP_PKEY_encrypt(ctx, out, &outlen, in.data, in.strlen) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_free("out");
        ERROR("EVP_PKEY_encrypt");
    }
    print("暗号化されたデータ: ", (uint8_t *)out, outlen);
    if (result->size >= outlen)
    {
        memcpy(result->data, out, outlen);
        result->strlen = outlen;
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

void RSA_decrypt_evp(RSA *priv_key, dynamic_mem_t in, dynamic_mem_t *out)
{
    print("暗号文: ", (uint8_t *)in.data, in.strlen);

    // 鍵をpkeyへ設定
    EVP_PKEY *pkey = EVP_PKEY_new();
    if (EVP_PKEY_set1_RSA(pkey, priv_key) <= 0)
    {
        EVP_PKEY_free(pkey);
        ERROR("EVP_PKEY_set1_RSA");
    }
    printf("RSA key size: %d\n", RSA_size(priv_key));

    // アルゴリズムの設定
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx)
    {
        ERROR("EVP_PKEY_CTX_new");
    }
    if (EVP_PKEY_decrypt_init(ctx) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        ERROR("EVP_PKEY_decrypt_init");
    }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        ERROR("EVP_PKEY_CTX_set_rsa_padding");
    }

    // 復号に必要な出力サイズを取得
    size_t outlen = 0;
    int error = 0;
    if ((error = EVP_PKEY_decrypt(ctx, NULL, &outlen, (const unsigned char *)in.data, (size_t)in.strlen)) <= 0)
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
    if ((error = EVP_PKEY_decrypt(ctx, out_buf, &outlen, (unsigned char *)in.data, in.strlen)) <= 0)
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
    print("復号されたデータ: ", (uint8_t *)out_buf, (int)outlen);
    if (out->size >= outlen)
    {
        memcpy(out->data, out_buf, outlen);
        out->strlen = outlen;
    }
    else
    {
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_free(out_buf);
        ERROR("memcpy");
    }
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    OPENSSL_free(out_buf);
}

int main()
{
    // 鍵生成
    rsa_key_create(KEY_SIZE);
    RSA *pub_key = get_rsa_key(PUBLIC_KEY_PEM, PUBLIC_KEY_FILE);
    RSA *priv_key = get_rsa_key(PRIVATE_KEY_PEM, PRIVATE_KEY_FILE);
    BIGNUM *pube, *pubn, *pubd;
    pube = BN_new();
    pubn = BN_new();
    pubd = BN_new();
    RSA_get0_key(pub_key, &pubn, &pube, NULL);
    RSA_get0_key(priv_key, NULL, NULL, &pubd);
    printf("RSA size: %d\n", RSA_size(pub_key));
    printf("pubn: %s\npube: %s\npubd: %s\n", BN_bn2hex(pubn), BN_bn2hex(pube), BN_bn2hex(pubd));

    // 共通鍵生成
    size_t shard_key_size = 256 / 8;
    unsigned char *shard_key = calloc(shard_key_size, sizeof(unsigned char));
    RAND_bytes(shard_key, shard_key_size);
    print("共通鍵: ", (uint8_t *)shard_key, (int)shard_key_size);

    // 暗号化
    puts("\n[暗号化]");
    dynamic_mem_t in;
    in.data = dynamic_new(&in, shard_key_size);
    memcpy(in.data, shard_key, shard_key_size);
    in.strlen = shard_key_size;
    dynamic_mem_t out;
    out.data = dynamic_new(&out, RSA_size(pub_key));
    printf("rsakey: %d\n", RSA_size(pub_key));
    RSA_encrypt_evp(pubn, pube, in, &out);
    // print("main 受け取り: ", (uint8_t *)out.data, out.strlen);

#if 1
    // 復号
    printf("\n[復号]\n");
    dynamic_mem_t dec_text; //復号文
    dynamic_mem_t enc_text; //暗号文
    enc_text.data = dynamic_new(&enc_text, out.strlen);
    dynamic_cpy(&enc_text, &out);
    printf("rsakey: %d\n", RSA_size(priv_key));
    dec_text.data = dynamic_new(&dec_text, enc_text.strlen);
    dec_text.strlen = 0;
    RSA_decrypt_evp(priv_key, enc_text, &dec_text);
    // print("main 復号文受け取り: ", (uint8_t *)dec_text.data, (int)dec_text.strlen);

    dynamic_free(&in);
    dynamic_free(&out);
    dynamic_free(&dec_text);
    dynamic_free(&enc_text);

#endif
    return 0;
}