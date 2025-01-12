#include "ilbuilder.h"

op_program_t* create_program() {
    op_program_t* ptr = (op_program_t*)malloc(sizeof(op_program_t));
    {
        ptr->length = 0;
        ptr->instructions = NULL;
    }
    return ptr;
}

void add_instruction(op_program_t* program, op_type_t type, int length, size_t* sizes, void* operands) {
    program->instructions = (op_inst_t**)realloc(
        program->instructions, 
        sizeof(op_inst_t*) * (program->length + 1)
    );

    op_inst_t* instruction = (op_inst_t*)malloc(sizeof(op_inst_t));
    {
        instruction->index = program->length;
        instruction->type = type;
        instruction->length = length;
        instruction->sizes = std::move(sizes);
        instruction->operands = std::move(operands);
    }

    program->instructions[program->length] = instruction;
    program->length++;
}
