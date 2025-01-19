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

// "hello, world!"
void* get_operand_from_string(char* str) {
    if (str == NULL || *str == '\0') {
        return NULL;
    }
    
    size_t len = strlen(str);
    if (str[0] == '"' && str[len-1] == '"') {
        str[len-1] = '\0';
        str++;
    }
    
    char* operand = (char*)malloc(strlen(str) + 1);
    strcpy(operand, str);
    return operand;
}

// %eax
void* get_operand_from_register(char* str) {
    int* operand = (int*)malloc(sizeof(int));
    memset(operand, 0, sizeof(int));
    
    if (str[0] == '%') str++;

    if (strcmp(str, "eax") == 0) *operand = 0;
    else if (strcmp(str, "ebx") == 0) *operand = 1;
    else if (strcmp(str, "ecx") == 0) *operand = 2;
    else if (strcmp(str, "edx") == 0) *operand = 3;
    else *operand = -1;
    
    return operand;
}

// [eax]
void* get_operand_from_memory(char* str) {
    int* operand = (int*)malloc(sizeof(int));
    memset(operand, 0, sizeof(int));
    
    str++;
    str[strlen(str)-1] = '\0';
    void* reg = get_operand_from_register(str);
    *operand = *(int*)reg;
    free(reg);
    
    return operand;
}

void* get_operand_from_auto(char* str) {
    if (str[0] == '"') {
        return get_operand_from_string(str);
    } else if (str[0] == '[') {
        return get_operand_from_memory(str);
    } else if (str[0] == '%' || isalpha(str[0])) {
        return get_operand_from_register(str);
    } else if ((str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) || str[strlen(str) - 1] == 'h') {
        return get_operand_from_hex(str);
    } else return get_operand_from_int(str);
}