#pragma once
#include <string>
#include <memory>
#include <netinet/in.h>

typedef std::string string;
typedef std::size_t size;
typedef signed long long Int64;
typedef signed int Int32;
typedef signed short Int16;
typedef signed char Int8;

typedef unsigned long long UInt64;
typedef unsigned int UInt32;
typedef unsigned short UInt16;
typedef unsigned char UInt8;

typedef unsigned char Byte;
typedef Byte ColourID;
typedef unsigned char ByteArray[256];
typedef string JsonTextComponent;
typedef Int64 UUID; // needs to be 128bit int
typedef string Identifier;
typedef string TextComponent;



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