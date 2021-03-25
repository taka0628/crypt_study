#include "RSA.h"

using namespace std;

rsa_c::rsa_c()
{
    this->private_key = RSA_new();
    this->public_key = RSA_new();
    this->key_size_bit = 0;
    this->key_size_byte = 0;
    this->isKeyGen = false;
}

rsa_c::rsa_c(const int key_size_bit)
{
    rsa_c::key_set(key_size_bit);
    this->private_key = RSA_new();
    this->public_key = RSA_new();
    this->isKeyGen = false;
}

rsa_c::~rsa_c()
{
    cout << ("RSA_free st") << endl;
    RSA_free(this->private_key);
    RSA_free(this->public_key);
    cout << ("RSA_free ed") << endl;
    this->key_size_bit = 0;
    this->key_size_byte = 0;
}

rsa_c::rsa_c(rsa_c &temp){}


void rsa_c::key_set(const int key_size_bit)
{
    if (key_size_bit <= 0)
    {
        ERROR("key_size_error");
        cerr << "key_size_bit: " << key_size_bit;
    }
    else
    {
        this->key_size_bit = key_size_bit;
        this->key_size_byte = key_size_bit / 8;
    }
}

int rsa_c::get_key_size_bit() const
{
    return this->key_size_bit;
}

int rsa_c::get_key_size_byte() const
{
    return this->key_size_byte;
}

void rsa_c::create_RSA(RSA *keypair, int pem_type, char *file_name)
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
        PEM_write_RSAPrivateKey(fp, keypair, NULL, NULL, get_key_size_bit(), NULL, NULL);
        fclose(fp);

        fp = fopen(file_name, "rb");
        PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL);
        fclose(fp);
    }
    RSA_free(rsa);
}

void rsa_c::set_public_key()
{
    FILE *fp = NULL;
    fp = fopen(PUBLIC_KEY_FILE, "rb");
    PEM_read_RSAPublicKey(fp, &this->public_key, NULL, NULL);
    fclose(fp);
}

void rsa_c::set_private_key()
{
    FILE *fp = NULL;
    fp = fopen(PRIVATE_KEY_FILE, "rb");
    PEM_read_RSAPrivateKey(fp, &this->private_key, NULL, NULL);
    fclose(fp);
}

RSA *rsa_c::get_public_key() const
{
    if (this->isKeyGen)
    {
        return this->public_key;
    }
    else
    {
        cerr << "鍵が生成されていません";
        return nullptr;
    }
}

RSA *rsa_c::get_private_key() const
{
    if (this->isKeyGen)
    {
        return this->private_key;
    }
    else
    {
        cerr << "鍵が生成されていません";
        return nullptr;
    }
}

void rsa_c::RSA_generate_key(const int key_size_bit)
{
    key_set(key_size_bit);
    BIGNUM *bn;
    bn = BN_new();
    BN_set_word(bn, RSA_F4);

    RSA *keypair;
    keypair = RSA_new();
    RSA_generate_key_ex(keypair, get_key_size_bit(), bn, NULL);
    create_RSA(keypair, PRIVATE_KEY_PEM, PRIVATE_KEY_FILE);
    create_RSA(keypair, PUBLIC_KEY_PEM, PUBLIC_KEY_FILE);
    BN_free(bn);
    RSA_free(keypair);
    rsa_c::set_private_key();
    rsa_c::set_public_key();
    this->isKeyGen = true;
}

void print(const char *header, uint8_t *buf, int size)
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

void rsa_c::RSA_public_encryption(const mem_c &plane_tex, mem_c &enc_text)
{
    mem_c *temp = new mem_c(this->get_key_size_byte());

    // 暗号化
    temp->len = RSA_public_encrypt(strlen(plane_tex.data), (unsigned char *)plane_tex.data, (unsigned char *)temp->data, rsa_c::get_public_key(), RSA_PKCS1_OAEP_PADDING);
    if (temp->len <= 0)
    {
        int error = ERR_get_error();
        char *str_error = (char *)malloc(256);
        str_error = (char *)ERR_reason_error_string(error);
        printf("%s\n", str_error);
        free(str_error);
        ERROR("RSA_public_encrypt");
        return;
    }

    print("暗号化されたデータ: ", (uint8_t *)temp->data, temp->len);
    printf("outlen: %ld\n", temp->len);
    puts("data cpy");
    enc_text.cpy(temp->data, temp->len);

    delete temp;
}

void rsa_c::RSA_private_decording(const mem_c &enc_text, mem_c &plane_text)
{
    mem_c temp(this->get_key_size_byte());
    cout << "enc_text.len: " << enc_text.len;
    temp.len = RSA_private_decrypt(enc_text.len, (const unsigned char *)enc_text.data, (unsigned char *)temp.data, this->get_private_key(), RSA_PKCS1_OAEP_PADDING);
    if (temp.len <= 0)
    {
        int error = ERR_get_error();
        char *str_error = (char *)malloc(256);
        str_error = (char *)ERR_reason_error_string(error);
        printf("%s\n", str_error);
        free(str_error);
        ERROR("RSA_private_decrypt");
    }

    // printf("復号されたデータ: %s\n", temp.data);
    print("復号されたデータ: ", (uint8_t *)temp.data, temp.len);
    if (temp.len <= plane_text.size)
    {
        plane_text.cpy(temp.data, static_cast<size_t>(temp.len));
    }
    else
    {
        ERROR("lengs error");
    }
}
