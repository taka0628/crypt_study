/**
 * rsacrypt.c
 *  RSA Encrypt/Decrypt & Sign/Verify Test Program for OpenSSL
 *  wrtten by blanclux
 *  This software is distributed on an "AS IS" basis WITHOUT WARRANTY OF ANY KIND.
 */
#include <stdio.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/objects.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#define KEYBIT_LEN 1024

static void
printHex(const char *title, const unsigned char *s, int len)
{
    int n;
    printf("%s:", title);
    for (n = 0; n < len; ++n)
    {
        if ((n % 16) == 0)
        {
            printf("\n%04x", n);
        }
        printf(" %02x", s[n]);
    }
    printf("\n");
}

int doCrypt(RSA *prikey, RSA *pubkey, unsigned char *data, int dataLen)
{
    int i;
    int encryptLen, decryptLen;
    unsigned char encrypt[1024], decrypt[1024];

    /* encrypt */
    encryptLen = RSA_public_encrypt(dataLen, data, encrypt, pubkey,
                                    RSA_PKCS1_OAEP_PADDING);
    /* print data */
    printHex("ENCRYPT", encrypt, encryptLen);
    printf("Encrypt length = %d\n", encryptLen);

    /* decrypt */
    decryptLen = RSA_private_decrypt(encryptLen, encrypt, decrypt, prikey,
                                     RSA_PKCS1_OAEP_PADDING);
    printHex("DECRYPT", decrypt, decryptLen);
    if (dataLen != decryptLen)
    {
        return 1;
    }
    for (i = 0; i < decryptLen; i++)
    {
        if (data[i] != decrypt[i])
        {
            return 1;
        }
    }

    return 0;
}

int doSign(RSA *prikey, RSA *pubkey, unsigned char *data, int dataLen)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned char sign[256];
    unsigned int signLen;
    int ret;

    SHA256(data, dataLen, hash);

    /* Sign */
    ret = RSA_sign(NID_sha256, hash, SHA256_DIGEST_LENGTH, sign,
                   &signLen, prikey);
    printHex("SIGN", sign, signLen);
    printf("Signature length = %d\n", signLen);
    printf("RSA_sign: %s\n", (ret == 1) ? "OK" : "NG");

    /* Verify */
    ret = RSA_verify(NID_sha256, hash, SHA256_DIGEST_LENGTH, sign,
                     signLen, pubkey);
    printf("RSA_Verify: %s\n", (ret == 1) ? "true" : "false");

    return ret;
}

int main(int argc, char *argv[])
{
    int ret;
    char *text = "The quick brown fox jumps over the lazy dog";
    RSA *prikey, *pubkey;
    unsigned char *data;
    unsigned int dataLen;
    char *p, *q, *n, *e, *d;
    char errbuf[1024];
    FILE *priKeyFile;

    if (argc > 2)
    {
        fprintf(stderr, "%s plainText\n", argv[0]);
        return 1;
    }
    if (argc == 1)
    {
        data = (unsigned char *)text;
        dataLen = strlen(text);
    }
    else
    {
        data = (unsigned char *)argv[1];
        dataLen = strlen(argv[1]);
    }

    ERR_load_crypto_strings();

    /* generate private key & public key */
    printf("< RSA Key Generation >\n");
    prikey = RSA_generate_key(KEYBIT_LEN, RSA_F4, NULL, NULL);
    if (prikey == NULL)
    {
        printf("RSA_generate_key: err = %s\n",
               ERR_error_string(ERR_get_error(), errbuf));
        return 1;
    }
    priKeyFile = fopen("RSAPriKey.pem", "w");
    if (priKeyFile == NULL)
    {
        perror("failed to fopen");
        return 1;
    }
    p = BN_bn2hex(prikey->p);
    q = BN_bn2hex(prikey->q);
    n = BN_bn2hex(prikey->n);
    e = BN_bn2hex(prikey->e);
    d = BN_bn2hex(prikey->d);
    printf("p = 0x%s\n", p);
    printf("q = 0x%s\n", q);
    printf("n = 0x%s\n", n);
    printf("e = 0x%s\n", e);
    printf("d = 0x%s\n", d);

    /* write private key to file (PEM format) */
    if (PEM_write_RSAPrivateKey(priKeyFile, prikey, NULL, NULL, 0,
                                NULL, NULL) != 1)
    {
        printf("PEM_write_RSAPrivateKey: err = %s\n",
               ERR_error_string(ERR_get_error(), errbuf));
        return 1;
    }

    /* copy public keys */
    pubkey = RSA_new();
    BN_hex2bn(&(pubkey->e), e);
    BN_hex2bn(&(pubkey->n), n);

    /* encrypt & decrypt */
    printf("\n< RSA Encrypt/Decrypt >\n");
    printHex("PLAIN", data, dataLen);

    ret = doCrypt(prikey, pubkey, data, dataLen);
    if (ret != 0)
    {
        printf("Encrypt/Decrypt Error.\n");
        return ret;
    }

    printf("\n< RSA Sign/verify >\n");
    ret = doSign(prikey, pubkey, data, dataLen);
    if (ret != 1)
    {
        printf("Sign/Verify Error.\n");
        return ret;
    }

    RSA_free(prikey);
    RSA_free(pubkey);
    OPENSSL_free(p);
    OPENSSL_free(q);
    OPENSSL_free(n);
    OPENSSL_free(e);
    OPENSSL_free(d);
    fclose(priKeyFile);

    return 0;
}