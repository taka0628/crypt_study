#include "mem.h"

using namespace std;

mem_c::mem_c(size_t size)
{
    this->size = size;
    this->data = new char[size];
    this->len = 0;
}

mem_c::~mem_c()
{
    this->size = 0;
    delete[] this->data;
    this->len = 0;
}

void mem_c::cpy(const char *data, const int size)
{
    if (this->size >= size)
    {
        memcpy(this->data, data, size);
        this->len = size;
    }
    else
    {
        cerr << "データ領域が足りません";
        exit(1);
    }
}

size_t mem_c::get_size() const
{
    return this->size;
}

size_t mem_c::get_len() const
{
    return this->len;
}