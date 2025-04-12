#include <network/VarIntLong.hpp>
// #include <Standards.hpp>
#include <Console.hpp>
#include <cstring>   // For memcpy
#include <vector>
#include <cstdint> // For int32_t, int64_t

// ==VarInt==
/*
VarInt::VarInt(int num) : value(static_cast<Int32>(num)) {}
*/
VarInt::VarInt() {
	value = 0;
	encode();
}

VarInt::VarInt(int32_t num) {
	value = num;
	encode();
}

VarInt::VarInt(void* packet) {
	value = decode(packet);
}

int32_t VarInt::getRawValue() {
	return value;
}


int VarInt::getValue() {
	return static_cast<int>(value);
}

int32_t VarInt::decode(void*& packet) {
	int32_t value = 0;
    int position = 0;
    uint8_t currentByte;
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

std::vector<uint8_t> VarInt::getBytes() const {
	return bytes;
}

void VarInt::encode() {
	int32_t value = this->value;
	do {
		uint8_t temp = value & SEGMENT_BITS;
		value >>= 7;
		if (value != 0) {
			temp |= CONTINUE_BIT;
		}
		bytes.push_back(temp);
	} while (value != 0);
}

// ==VarLong==
/*
VarLong::VarLong(long long num) : value(static_cast<Int64>(num)) {}
*/
VarLong::VarLong() {
	value = 0;
	encode();
}

VarLong::VarLong(int64_t num) {
	value = num;
	encode();
}

VarLong::VarLong(void* packet) {
	value = decode(packet);
}

int64_t VarLong::getRawValue() {
	return value;
}

long long VarLong::getValue() {
	return static_cast<long long>(value);
}

int64_t VarLong::decode(void*& packet) {
	int64_t value = 0;
    int position = 0;
    uint8_t currentByte;
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

std::vector<uint8_t> VarLong::getBytes() const {
	return bytes;
}

void VarLong::encode() {
	int64_t value = this->value;
	do {
		uint8_t temp = value & SEGMENT_BITS;
		value >>= 7;
		if (value != 0) {
			temp |= CONTINUE_BIT;
		}
		bytes.push_back(temp);
	} while (value != 0);
}