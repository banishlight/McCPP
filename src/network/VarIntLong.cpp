#include <network/VarIntLong.hpp>
#include <Standards.hpp>
#include <Console.hpp>
#include <cstring>   // For memcpy
#include <vector>

// When Standards moves to fixed int sizes, uncomment constructors

// ==VarInt==
/*
VarInt::VarInt(int num) : value(static_cast<Int32>(num)) {}
*/
VarInt::VarInt(Int32 num) : value(num) {}

VarInt::VarInt(void* packet) {
	value = decode(packet);
}

Int32 VarInt::getRawValue() {
	return value;
}


int VarInt::getValue() {
	return static_cast<int>(value);
}

Int32 VarInt::decode(void*& packet) {
	Int32 value = 0;
    int position = 0;
    Int8 currentByte;
	while(true) {
		std::memcpy(&currentByte, packet, 1);
		bytes.push_back(currentByte);
		packet = static_cast<uint8_t*>(packet) + 1;
		value |= (currentByte & SEGMENT_BITS) << position;
		if ((currentByte & CONTINUE_BIT) == 0) {
            break;
        }
		position += 7;
		if (position >= 32) {
            Console::getConsole().Error("VarInt is too big");
        }
	}
	return value;
}

std::vector<Int8> VarInt::getBytes() {
	return bytes;
}

// ==VarLong==
/*
VarLong::VarLong(long long num) : value(static_cast<Int64>(num)) {}
*/
VarLong::VarLong(Int64 num) : value(num) {}

VarLong::VarLong(void* packet) {
	value = decode(packet);
}

Int64 VarLong::getRawValue() {
	return value;
}

long long VarLong::getValue() {
	return static_cast<long long>(value);
}

Int64 VarLong::decode(void*& packet) {
	Int64 value = 0;
    int position = 0;
    Int8 currentByte;
	while(true) {
		std::memcpy(&currentByte, packet, 1);
		bytes.push_back(currentByte);
		packet = static_cast<uint8_t*>(packet) + 1;
		value |= (currentByte & SEGMENT_BITS) << position;
		if ((currentByte & CONTINUE_BIT) == 0) {
            break;
        }
		position += 7;
		if (position >= 64) {
            Console::getConsole().Error("VarInt is too big");
        }
	}
	return value;
}

std::vector<Int8> VarLong::getBytes() {
	return bytes;
}