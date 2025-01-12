#ifndef __OPCODES_H__
#define __OPCODES_H__

#include "stdafx.h"

typedef enum op_type_t : unsigned char {
  PUSH    = 0x00,                           // Pushes an item to the stack  (FILO)
  POP     = 0x01,                           // Pops an item from the stack  (FILO)
  ADD     = 0x03,
  SUB     = 0x04,
  MUL     = 0x05,
  DIV     = 0x06,
  OR      = 0x07,
  XOR     = 0x08,
  AND     = 0x09,
  CMP     = 0x0A,
  JMP     = 0x0B,
  JE      = 0x0C,
  JNE     = 0x0D, 
  INT     = 0x0E,                           // Interrupt
};

typedef struct op_inst_t {
  int index;
  unsigned char type;
  int length;
  size_t* sizes;
  void* operands;
};

#endif // __OPCODES_H__