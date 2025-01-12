#ifndef __BINARY_H__
#define __BINARY_H__

#include "stdafx.h"
#include "log.h"

static constexpr int MAX_BINARY_COMMON_SIZE = 12 * 8 * 8 * 4 * 2;

typedef struct binaryc_t {
  size_t position;
  size_t length;
  bool can_write;
  unsigned char memory[MAX_BINARY_COMMON_SIZE];

  bool is_out_of_bounds(size_t move_size);
  size_t append(const unsigned char* data, size_t length);
  size_t read(unsigned char* dest, size_t length);
  void reset();
  
  const unsigned char* get();
};

typedef struct binaryp_t : public binaryc_t {
  void write_int8(char i);
  void write_int16(short i);
  void write_int32(int i);
  void write_int64(long i);
  void write_uint8(unsigned char i);
  void write_uint16(unsigned short i);
  void write_uint32(unsigned int i);
  void write_uint64(unsigned long i);
  size_t write_string(const char* str);

  char read_int8();
  short read_int16();
  int read_int32();
  long read_int64();
  unsigned char read_uint8();
  unsigned short read_uint16();
  unsigned int read_uint32();
  unsigned long read_uint64();
  size_t read_string(unsigned char* buffer);
};

binaryp_t* create_binary_stream(bool can_write);

#endif // __BINARY_H__