#ifndef ___RSA_h
#define ___RSA_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#include <cstdlib>
#include <cstdio>
#include <new>
#include <iostream>
#include <fstream>
#include <vector>

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/dh.h>

#include "mem.h"

#define PRIVATE_KEY_FILE "private_key"
#define PUBLIC_KEY_FILE "public_key"
#define PUBLIC_KEY_PEM 1
#define PRIVATE_KEY_PEM 0

#define ERROR(comment) \
    printf("[ERROR]\n\t%s: %d\n\t%s\n", __func__, __LINE__, comment);

class rsa_c
{
private:
    RSA *private_key;
    RSA *public_key;
    int key_size_byte;
    int key_size_bit;
    bool isKeyGen;

    void key_set(const int key_size_bit);
    RSA *get_private_key() const;
    void set_public_key();
    void set_private_key();
    rsa_c(rsa_c &temp);

public:
    rsa_c();
    rsa_c(const int key_size_bit);
    int get_key_size_bit() const;
    int get_key_size_byte() const;
    RSA *get_public_key() const;
    void RSA_generate_key(int key_size_bit);
    void RSA_public_encryption(const mem_c &plane_tex, mem_c &enc_text);
    void RSA_private_decording(const mem_c &enc_text, mem_c &plane_text);
    void create_RSA(RSA *keypair, int mode, char *output_filename);

    ~rsa_c();
};

#endif