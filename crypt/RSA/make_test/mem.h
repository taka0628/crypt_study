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
public:
    char *data;
    size_t size;
    size_t len;
    mem_c(size_t size);
    ~mem_c();
    void cpy(const char *data, const int size);
    size_t get_size() const;
    size_t get_len() const;
};

class mem
{
private:
    /* data */
public:
    mem(/* args */);
    ~mem();
};


#endif