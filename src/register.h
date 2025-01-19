#ifndef __REGISTER_H__
#define __REGISTER_H__

#include "stdafx.h"

typedef struct register_t {
    unsigned char id;
    size_t size;
    void* data;
};

static constexpr int MAX_REGISTER_SIZE = 20;
static register_t all_registers[MAX_REGISTER_SIZE];

void add_register(unsigned char id, size_t size);
register_t* get_register(unsigned char id);
void fast_set_register(unsigned char id, void* data, size_t size);

// NOTE: 'size_t* size' is an OUT parameter
void* fast_get_register(unsigned char* id, size_t* size);

#endif // __REGISTER_H__