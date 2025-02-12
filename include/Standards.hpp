#pragma once
#include <string>
#include <memory>
#include <netinet/in.h>
#include <vector>
#include <cstdint> 


typedef std::string string;
typedef std::size_t size;
typedef int64_t Int64;
typedef int32_t Int32;
typedef int16_t Int16;
typedef int8_t Int8;

typedef uint64_t UInt64;
typedef uint32_t UInt32;
typedef uint16_t UInt16;
typedef uint8_t UInt8;

// Possibly unnecessary:
typedef uint8_t Byte;
typedef Byte ColourID;
typedef unsigned char ByteArray[256];
typedef string JsonTextComponent;
typedef Int64 UUID[2]; // needs to be 128bit int
typedef string Identifier;
typedef string TextComponent;

typedef struct {
  int size;
  int id;
  int dsize;
  std::vector<Byte> data;
} Packet;

const string PROTOCOL_VERSION = "764";
const string SERVER_VERSION = "1.20.2";

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')


void decodePosition(Int64 position, Int32 *x, Int32 *y, Int32 *z);
