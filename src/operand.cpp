#include "operand.h"

// 20
void* get_operand_from_int(char* str) {
    int* operand = (int*)malloc(sizeof(int));
    memset(operand, 0, sizeof(int));

    *operand = std::stoi(str);
    return operand;
}


// 0x33, 21h
void* get_operand_from_hex(char* str) {
    int* operand = (int*)malloc(sizeof(int));
    memset(operand, 0, sizeof(int));
    
    if (str == NULL || *str == '\0') {
        return operand;
    }

    while (*str && isspace(*str)) {
        str++;
    }

    int base = 16;
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
    } else {
        char* h_pos = strchr(str, 'h');
        if (h_pos != NULL) {
            *h_pos = '\0';
        } else base = 10;
    }

    char* endptr;
    long value = strtol(str, &endptr, base);
    if (endptr == str || (endptr != NULL && *endptr != '\0' && !isspace(*endptr))) {
        *operand = 0;
    } else *operand = (int)value;

    return operand;
}