#ifndef __ILBUILDER_H__
#define __ILBUILDER_H__

#include "stdafx.h"
#include "log.h"
#include "opcodes.h"

typedef struct op_program_t {
  int length;
  op_inst_t** instructions;
};

op_program_t* create_program();
void add_instruction(op_program_t* program, op_type_t type, int length, size_t* sizes, void* operands);

#endif // __ILBUILDER_H__