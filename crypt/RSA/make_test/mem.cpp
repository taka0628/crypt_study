#include "mem.h"

using namespace std;

mem_c::mem_c(size_t size)
{
    this->size = size;
    this->data = new char[size];
    for (size_t i = 0; i < size; i++)
    {
        this->data[i] = 0;
    }
    this->len = 0;
}

mem_c::~mem_c()
{
    cout << ("mem_clear st") << endl;
    this->size = 0;
    if (this->data != nullptr)
    {
        delete[] this->data;
        this->data = nullptr;
    }
    this->len = 0;
    cout << ("mem_clear ed") << endl;
}

mem_c::mem_c(const mem_c &temp){}

void mem_c::cpy(const char *data, const size_t size)
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