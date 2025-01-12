#include "binary.h"

bool binaryc_t::is_out_of_bounds(size_t move_size) {
    return this->position + move_size > MAX_BINARY_COMMON_SIZE;
}

size_t binaryc_t::append(const unsigned char* data, size_t length) {
    if (!this->can_write) {
      LOG_MSG("[-] Cannot append() because the stream is not writable");
      return 0;
    }

    if (is_out_of_bounds(length)) {
      LOG_MSG("[-] append() is out of bounds (position: " << this->position << ", move size: " << this->position + length << ", max: " << MAX_BINARY_COMMON_SIZE << ")");
      return 0;
    }

    memcpy(&memory[position], data, length);
    position += length;
    this->length += length;

    return length;
  }

size_t binaryc_t::read(unsigned char* dest, size_t length) {
    if (length < 0) {
        LOG_MSG("[-] read() received a negative length");
        return 0;
    }

    if (is_out_of_bounds(length)) {
        LOG_MSG("[-] read() is out of bounds (position: " << position 
                << ", move size: " << position + length 
                << ", max: " << MAX_BINARY_COMMON_SIZE << ")");
        return 0;
    }

    memcpy(dest, &memory[position], length);
    position += length; 
    return length;
}

void binaryc_t::reset() { this->position = 0; }

// Returns a dangling pointer. Remember to free after use.
const unsigned char* binaryc_t::get() { 
    unsigned char* ptr = (unsigned char*)malloc(sizeof(unsigned char) * this->length);
    memcpy(ptr, this->memory, this->length);
    return ptr;
}

void binaryp_t::write_int8(char i) {
    append(reinterpret_cast<unsigned char*>(&i), sizeof(char));
}

void binaryp_t::write_int16(short i) {
    append(reinterpret_cast<unsigned char*>(&i), sizeof(short));
}

void binaryp_t::write_int32(int i) {
    append(reinterpret_cast<unsigned char*>(&i), sizeof(int));
}

void binaryp_t::write_int64(long i) {
    append(reinterpret_cast<unsigned char*>(&i), sizeof(long));
}

void binaryp_t::write_uint8(unsigned char i) {
    append(reinterpret_cast<unsigned char*>(&i), sizeof(unsigned char));
}

void binaryp_t::write_uint16(unsigned short i) {
    append(reinterpret_cast<unsigned char*>(&i), sizeof(unsigned short));
}

void binaryp_t::write_uint32(unsigned int i) {
    append(reinterpret_cast<unsigned char*>(&i), sizeof(unsigned int));
  }

void binaryp_t::write_uint64(unsigned long i) {
    append(reinterpret_cast<unsigned char*>(&i), sizeof(unsigned long));
}

size_t binaryp_t::write_string(const char* str) {
    const size_t length = strlen(str);
    write_int64(static_cast<long>(length));
    append(reinterpret_cast<const unsigned char*>(str), length);
    return length;
}

char binaryp_t::read_int8() {
    char value;
    read(reinterpret_cast<unsigned char*>(&value), sizeof(char));
    return value;
}

short binaryp_t::read_int16() {
    short value;
    read(reinterpret_cast<unsigned char*>(&value), sizeof(short));
    return value;
}

int binaryp_t::read_int32() {
    int value;
    read(reinterpret_cast<unsigned char*>(&value), sizeof(int));
    return value;
}

long binaryp_t::read_int64() {
    long value;
    read(reinterpret_cast<unsigned char*>(&value), sizeof(long));
    return value;
}

unsigned char binaryp_t::read_uint8() {
    unsigned char value;
    read(reinterpret_cast<unsigned char*>(&value), sizeof(unsigned char));
    return value;
}

unsigned short binaryp_t::read_uint16() {
    unsigned short value;
    read(reinterpret_cast<unsigned char*>(&value), sizeof(unsigned short));
    return value;
}

unsigned int binaryp_t::read_uint32() {
    unsigned int value;
    read(reinterpret_cast<unsigned char*>(&value), sizeof(unsigned int));
    return value;
}

unsigned long binaryp_t::read_uint64() {
    unsigned long value;
    read(reinterpret_cast<unsigned char*>(&value), sizeof(unsigned long));
    return value;
}

size_t binaryp_t::read_string(unsigned char* buffer) {
    const size_t length = static_cast<size_t>(read_int64());
    return this->read(buffer, length);
}

binaryp_t* create_binary_stream(bool can_write) {
    binaryp_t* stream = (binaryp_t*)malloc(sizeof(binaryp_t));
    if (stream == nullptr) {
        LOG_MSG("[-] Failed to allocate memory for binaryc_t stream");
        return NULL;
    }

    stream->position = 0;
    stream->can_write = can_write;
    memset(stream->memory, 0, sizeof(stream->memory));
    return stream;
}