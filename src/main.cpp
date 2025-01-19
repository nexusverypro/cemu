//
// Copyright (c) nexusverypro, 2025
//

#include "stdafx.h"
#include "log.h"
#include "binary.h"
#include "opcodes.h"
#include "ilbuilder.h"

static constexpr int MEMORY_SIZE = 1024 * 8 * 4 * 2;
static constexpr int MEMORY_PADDING_SIZE = 16 * 16 * 2 * 4 * 2;
static constexpr int STACK_SIZE = 1024;

static constexpr int REGISTER_SIZE  = 14;
static constexpr int R0             = 0x00;
static constexpr int R1             = 0x01;
static constexpr int R2             = 0x02;
static constexpr int R3             = 0x03;
static constexpr int R4             = 0x04;
static constexpr int R5             = 0x05;
static constexpr int R6             = 0x06;
static constexpr int R7             = 0x07;
static constexpr int R8             = 0x08;
static constexpr int PC             = 0x09;     // current program counter
static constexpr int SP             = 0x0A;     // current stack location
static constexpr int PX             = 0x0B;     // max program counter length
static constexpr int ZF             = 0x0C;     // zero flag (used in CMP)
static constexpr int SF             = 0x0D;     // sign flag (used in compare, or jump ifs)

struct cpu_info_t {
    unsigned int total_run_cycles;
    unsigned int program_counter_lower_bound;
    unsigned int program_counter_higher_bound;
};

struct cpu_t {
    op_program_t* program;
    cpu_info_t* info;
    unsigned char memory[MEMORY_SIZE];
    unsigned int registers[REGISTER_SIZE];

#pragma region Memory
    bool asm_check_if_free_memory(int size, int* idx) {
        // traverse memory space to check any memory starting
        // with [0x0F, 0xFF, 0x01] (0x01 indicates free memory space)
        for (int i = 0; i < MEMORY_SIZE; ++i) {
            if ((i + 2 + size < MEMORY_SIZE) && memory[i] == 0x0F && 
                memory[i + 1] == 0xFF && memory[i + 2] == 0x01) {
                (*idx) = i + 3;
                return true;
            }
        }

        LOG_MSG("[D] No space found for memory of size : " << size);
        (*idx) = 0;
        return false;
    }

    bool asm_change_free_memory_bit(int idx, unsigned char bit) {
        if (idx >= MEMORY_SIZE) return false;
        memory[idx - 1] = 0x02;
        return true;
    }

    bool asm_misvalid(int idx) {
        return (idx >= 0 && idx <= MEMORY_SIZE);
    }

    void* asm_mofaddr(int idx) {
        return NULL;
    }

    void* asm_malloc(int size) {
        int idx = 0;
        if (!asm_check_if_free_memory(size, &idx))
            return NULL;
        if (!asm_change_free_memory_bit(idx, 0x02))
            return NULL;

        memory[idx - 3] = (size >> 8) & 0xFF;  // size (high byte)
        memory[idx - 2] = size & 0xFF;         // size (low byte)
        return &memory[idx];
    }

    void asm_mfree(void* ptr) {
        if (ptr == NULL) {
            LOG_MSG("[D] Invalid pointer provided to free");
            return;
        }

        unsigned char* p = static_cast<unsigned char*>(ptr);
        int idx = p - memory;
        if (idx < 0 || idx >= MEMORY_SIZE) {
            LOG_MSG("[D] Invalid pointer range");
            return;
        }

        int size = (memory[idx - 3] << 8) | memory[idx - 2];

        memory[idx - 3] = 0x0F;
        memory[idx - 2] = 0xFF;
        if (!asm_change_free_memory_bit(idx, 0x01)) {
            return;
        }
    }

    int asm_maddrof(void* ptr) {
        if (ptr == NULL) {
            LOG_MSG("[D] Invalid pointer provided to get address of");
            return 0;
        }

        unsigned char* p = static_cast<unsigned char*>(ptr);
        return std::max(0, static_cast<int>(p - memory));
    }

    void dump_memory(int startIndex, int chunkLimit) {
        const int CHUNK_SIZE = 15;
        if (startIndex < 0 || startIndex >= MEMORY_SIZE) {
            LOG_MSG("[ERROR] Invalid startIndex: " << startIndex);
            return;
        }

        int endIndex = startIndex + (chunkLimit * CHUNK_SIZE);
        if (endIndex > MEMORY_SIZE) {
            endIndex = MEMORY_SIZE;
        }

        LOG_MSG("======= MEM DUMP (" << (endIndex - startIndex) << " TOTAL) =======");
        for (int i = startIndex; i < endIndex; i += CHUNK_SIZE) {
            printf("0x%08X: ", i);
            for (int j = 0; j < CHUNK_SIZE; ++j) {
                if (i + j < endIndex) {
                    printf("%02X ", memory[i + j]);
                } else {
                    printf("   ");
                }
            }

            printf(" | ");
            for (int j = 0; j < CHUNK_SIZE; ++j) {
                if (i + j < endIndex) {
                    unsigned char byte = memory[i + j];
                    if (isprint(byte)) {
                        printf("%c", byte);
                    } else {
                        printf("~");
                    }
                }
            }
            printf("\n");
        }
        LOG_MSG("===============================================");
    }
#pragma endregion

#pragma region Stack

    void asm_stack_push(int value) {
        if (registers[SP] >= STACK_SIZE) {
            LOG_MSG("[ERROR] Stack overflow");
            return;
        }
        
        memory[registers[SP]] = (value >> 24) & 0xFF;
        memory[registers[SP] + 1] = (value >> 16) & 0xFF;
        memory[registers[SP] + 2] = (value >> 8) & 0xFF;
        memory[registers[SP] + 3] = value & 0xFF;
        registers[SP] += 4;
    }

    int asm_stack_pop() {
        if (registers[SP] < 4) {
            LOG_MSG("[ERROR] Stack underflow");
            return 0;
        }
        
        registers[SP] -= 4;
        return (memory[registers[SP]] << 24) |
            (memory[registers[SP] + 1] << 16) |
            (memory[registers[SP] + 2] << 8) |
            memory[registers[SP] + 3];
    }

#pragma endregion

    void initialize_memory() {
        // clean memory to fresh slate
        for (int i = 0; i < MEMORY_SIZE; ++i) {
            memory[i] = 0x00;
        }

        // init stack
        for (int i = 0; i < STACK_SIZE; ++i) {
            memory[i] = 0x00;
        }

        // init mem
        for (int i = STACK_SIZE; i < MEMORY_SIZE; ++i) {
            // init blocks of mem
            if (i + 2 < MEMORY_SIZE && (memory[i] != 0x0F && memory[i + 1] != 0xFF)) {
                memory[i] = 0x0F;
                memory[i + 1] = 0xFF;
                memory[i + 2] = 0x01;
                i += MEMORY_PADDING_SIZE;
            }
        }

        LOG_MSG("[+] Initialized memory");
    }

    void initialize_registers() {
        // clean registers to fresh slate
        for (int i = 0; i < REGISTER_SIZE; ++i) {
            registers[i] = 0x00;
        }

        registers[SP] = 0x00;
        LOG_MSG("[+] Initialized registers");
    }

    void initialize_bytecode() {
        auto stream_ptr = create_binary_stream(true);
        if (stream_ptr == NULL) {
            LOG_MSG("[D] Failed to create binary stream");
            return;
        }

        for (int i = 0; i < program->length; ++i) {
            auto inst_ptr = program->instructions[i];
            stream_ptr->write_uint8(inst_ptr->type);
            stream_ptr->write_int32(inst_ptr->length);
            for (int j = 0; j < inst_ptr->length; ++j) {
                size_t operand_size = inst_ptr->sizes[j];
                void* operand = (char*)inst_ptr->operands + j * operand_size;
                unsigned char* operand_buffer = static_cast<unsigned char*>(operand);
                stream_ptr->append(operand_buffer, operand_size);
            }
        }

        char* bytecode_ptr = (char*)asm_malloc(stream_ptr->length);
        memcpy(bytecode_ptr, stream_ptr->get(), stream_ptr->length);

        registers[PC] = asm_maddrof(bytecode_ptr);
        registers[PX] = (registers[PC] + stream_ptr->length);
        this->info->program_counter_lower_bound = registers[PC];
        this->info->program_counter_higher_bound = registers[PX]; 

        LOG_MSG("[+] Initialized bytecode (" << program->length << " instructions written)");
        free(stream_ptr);
    }


    void initialize(op_program_t* program) { 
        this->program = program;
        this->info = (cpu_info_t*)malloc(sizeof(cpu_info_t));
        memset(this->info, 0, sizeof(cpu_info_t));

        initialize_memory();
        initialize_registers();
        initialize_bytecode();
    }

    template<typename T> T get_operand(const unsigned char* bytecode, size_t& pc) {
        T operand;
        memcpy(&operand, &bytecode[pc], sizeof(T));
        pc += sizeof(T);
        return operand;
    }

    void validate() {
        if (registers[SP] > STACK_SIZE)
            throw std::runtime_error("Register 'SP' (Stack Pointer) is out of bounds. The max size is 1024.");
    }
    
    // execute() and execute_inst() should be tied together
    // meaning that execute_inst() should be able to control
    // the next location of memory to execute, whereas now it can
    // only execute each memory code sequentially.
    // 
    // this can be catastropic because it will also attempt
    // to execute operands as opcodes. not good :(
    // FIXED: 09/01/2025 nexusverypro
    // FIXME(FIXED): still not working? 10/01/2025 nexusverypro
    void execute_inst(binaryp_t* stream) {
        validate(); // validate our cpu

        this->info->total_run_cycles++;
        registers[PC] = this->info->program_counter_lower_bound + stream->position;

        unsigned char op_code = stream->read_uint8();
        const int length = stream->read_int32();

        // validate that we are still in bounds
        if (registers[PC] >= std::min(registers[PX], this->info->program_counter_higher_bound))
            return;
        LOG_MSG("[+] -> Executing opcode: " << (int)op_code);

        switch (op_code) {
            case PUSH: {
                if (length == 1) {
                    int value = stream->read_int32();
                    asm_stack_push(value);
                }
                break;
            }

            case POP: {
                if (length == 1) {
                    int registerIdx = stream->read_int32();
                    int value = asm_stack_pop();
                    registers[registerIdx] = value;
                }
                break;
            }

            case ADD: {
                if (length == 2) {
                    int lhsRegisterIdx = stream->read_int32();
                    int rhsRegisterIdx = stream->read_int32();

                    int value = registers[lhsRegisterIdx] + registers[rhsRegisterIdx];
                    registers[lhsRegisterIdx] = value;
                }
                break;
            }

            case SUB: {
                if (length == 2) {
                    int lhsRegisterIdx = stream->read_int32();
                    int rhsRegisterIdx = stream->read_int32();

                    int value = registers[lhsRegisterIdx] - registers[rhsRegisterIdx];
                    registers[lhsRegisterIdx] = value;
                }
                break;
            }

            case MUL: {
                if (length == 2) {
                    int lhsRegisterIdx = stream->read_int32();
                    int rhsRegisterIdx = stream->read_int32();

                    int value = registers[lhsRegisterIdx] * registers[rhsRegisterIdx];
                    registers[lhsRegisterIdx] = value;
                }
                break;
            }

            case DIV: {
                if (length == 2) {
                    int lhsRegisterIdx = stream->read_int32();
                    int rhsRegisterIdx = stream->read_int32();

                    int value = registers[lhsRegisterIdx] / registers[rhsRegisterIdx];
                    registers[lhsRegisterIdx] = value;
                }
                break;
            }

            case OR: {
                if (length == 2) {
                    int lhsRegisterIdx = stream->read_int32();
                    int rhsRegisterIdx = stream->read_int32();

                    int value = registers[lhsRegisterIdx] | registers[rhsRegisterIdx];
                    registers[lhsRegisterIdx] = value;
                }
                break;
            }

            case XOR: {
                if (length == 2) {
                    int lhsRegisterIdx = stream->read_int32();
                    int rhsRegisterIdx = stream->read_int32();
                    
                    int value = registers[lhsRegisterIdx] ^ registers[rhsRegisterIdx];
                    registers[lhsRegisterIdx] = value;
                }
                break;
            }

            case JMP: {
                if (length == 1) {
                    int jumpLabelIdx = stream->read_int32();
                    if (jumpLabelIdx >= 0 && jumpLabelIdx <= (this->info->program_counter_higher_bound - this->info->program_counter_lower_bound)) {
                        stream->position = jumpLabelIdx;
                    }
                }
                break;
            }

            default: {
                LOG_MSG("[-] -> Unknown opcode: " << (int)op_code);
                break;
            }
        }
    }

    void execute() {
        binaryp_t* stream_ptr = create_binary_stream(true);
        if (stream_ptr == NULL) {
            LOG_MSG("[D] Failed to create binary stream");
            return;
        }

        stream_ptr->append(
            (unsigned char*)&memory[registers[PC]],
            registers[PX] - registers[PC]
        );
        stream_ptr->reset();
        stream_ptr->can_write = false;

        // TODO(FIXED): loop immedately ends because PC is greater than
        // PX. might be a syncronization issue, because PC is always
        // memory space. PC is supposed to be the index of memory
        // while PX is supposed to be the length of total bytes.
        // FIXED: 10/01/2025 nexusverypro
        while (true) {
            if (registers[PC] >= std::min(registers[PX], this->info->program_counter_higher_bound)) {
                LOG_MSG("[+] Program execution complete: " << this->info->total_run_cycles << " total cycles");
                break;
            }

            execute_inst(stream_ptr);
        }

        free(stream_ptr);
    }
};

int main(int argc, char* argv[]) {
    auto program_ptr = create_program();

    // PUSH 0x01 (int)
    {
        int* operands = (int*)malloc(sizeof(int) * 1);
        operands[0] = 25;
        size_t* sizes = (size_t*)malloc(sizeof(size_t) * 1);
        sizes[0] = sizeof(int);
        add_instruction(program_ptr, PUSH, 1, sizes, operands);
    }

    // POP r5
    {
        int* operands = (int*)malloc(sizeof(int) * 1);
        operands[0] = R5;
        size_t* sizes = (size_t*)malloc(sizeof(size_t) * 1);
        sizes[0] = sizeof(int);
        add_instruction(program_ptr, POP, 1, sizes, operands);
    }

    // INT 21h
    {
        int* operands = (int*)malloc(sizeof(int) * 1);
        operands[0] = 0x21;
        size_t* sizes = (size_t*)malloc(sizeof(size_t) * 1);
        sizes[0] = sizeof(int);
        add_instruction(program_ptr, INT, 1, sizes, operands);
    }

    auto cpu = (cpu_t*)malloc(sizeof(cpu_t));
    cpu->initialize(program_ptr);
    cpu->execute();

    return EXIT_FAILURE;
}