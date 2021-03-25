#ifndef ___MEM_H
#define ___MEM_H

#include <new>
#include <memory.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>

class mem_c
{
private:
    mem_c(const mem_c &temp);

public:
    char *data;
    size_t size;
    size_t len;
    mem_c(size_t size);
    ~mem_c();
    void cpy(const char *data, const size_t size);
    size_t get_size() const;
    size_t get_len() const;
};

#endif