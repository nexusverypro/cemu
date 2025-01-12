#ifndef __OPERAND_H__
#define __OPERAND_H__

#include "stdafx.h"
#include "log.h"

typedef enum operand_type_t : unsigned char {
    INT         = 0x00,
    HEX         = 0x01,
    STRING      = 0x02,
    REGISTER    = 0x03,
    MEMORY      = 0x04,

    NONE        = 0xFF,
};

void* get_operand_from_int(char* str);                          // 20, 'h'
void* get_operand_from_hex(char* str);                          // 0x21, 21h
void* get_operand_from_string(char* str);                       // "hello world"
void* get_operand_from_register(char* str);                     // %eax
void* get_operand_from_memory(char* str);                       // [eax]
void* get_operand_from_auto(char* str);

#endif // __OPERAND_H__