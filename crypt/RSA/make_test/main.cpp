#include "RSA.h"

void print_h(const char *header, uint8_t *buf, int size)
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
// bool get_DH_key(DH *dh, )

int main()
{
    rsa_c rsa;
    rsa.RSA_generate_key(2048);

    size_t rand_size = 256 / 8;
    unsigned char *rand_buf = (unsigned char *)calloc(1024, sizeof(unsigned char));
    if (RAND_bytes((unsigned char *)rand_buf, rand_size) <= 0)
    {
        int error = ERR_get_error();
        char *str_error = (char *)malloc(256);
        str_error = (char *)ERR_reason_error_string(error);
        printf("%s\n", str_error);
        free(str_error);
        ERROR("RAND_bytes");
    }
    print_h("RAND: ", (uint8_t *)rand_buf, rand_size);

    // 暗号化
    // mem_c in(rsa.get_key_size_byte());
    // mem_c out(rsa.get_key_size_byte());
    mem_c *in = new mem_c(rsa.get_key_size_byte());
    mem_c *out = new mem_c(rsa.get_key_size_byte());
    in->cpy((char *)rand_buf, rand_size);
    print_h("in: ", (uint8_t *)in->data, in->len);

    rsa.RSA_public_encryption(*in, *out);
    print_h("main 受け取り: ", (uint8_t *)out->data, out->len);
    delete in;

    mem_c dec(rsa.get_key_size_byte());

    // rsa.RSA_private_decording(*out, dec);
    // print_h("main 復号: ", (uint8_t *)dec.data, dec.len);

    delete out;

    return 0;
}