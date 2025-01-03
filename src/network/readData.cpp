#include <Standards.hpp>
#include <cstring>
#include <cstdint> // for uint8_t

string readString(void* packet, int max) {
    
}

template <typename T>
T readData(void*& data) {
    T value;
    std::memcpy(&value, data, sizeof(T));
    data = static_cast<uint8_t*>(data) + sizeof(T);
    return value;
}

// Unsigned Int's

template<>
UInt8 readData<UInt8>(void*& data) {
    UInt8 value;
    std::memcpy(&value, data, sizeof(UInt8));
    // might need to convert network to host byte order
    data = static_cast<uint8_t*>(data) + sizeof(UInt8);
    return value;
}

template<>
UInt16 readData<UInt16>(void*& data) {
    UInt16 value;
    std::memcpy(&value, data, sizeof(UInt16));
    // might need to convert network to host byte order
    data = static_cast<uint8_t*>(data) + sizeof(UInt16);
    return value;
}

template<>
UInt32 readData<UInt32>(void*& data) {
    UInt32 value;
    std::memcpy(&value, data, sizeof(UInt32));
    // might need to convert network to host byte order
    data = static_cast<uint8_t*>(data) + sizeof(UInt32);
    return value;
}

template<>
UInt64 readData<UInt64>(void*& data) {
    UInt64 value;
    std::memcpy(&value, data, sizeof(UInt64));
    // might need to convert network to host byte order
    data = static_cast<uint8_t*>(data) + sizeof(UInt64);
    return value;
}

// Signed Int's

template<>
Int8 readData<Int8>(void*& data) {
    Int8 value;
    std::memcpy(&value, data, sizeof(Int8));
    // might need to convert network to host byte order
    data = static_cast<uint8_t*>(data) + sizeof(Int8);
    return value;
}

template<>
Int16 readData<Int16>(void*& data) {
    Int16 value;
    std::memcpy(&value, data, sizeof(Int16));
    // might need to convert network to host byte order
    data = static_cast<uint8_t*>(data) + sizeof(Int16);
    return value;
}

template<>
Int32 readData<Int32>(void*& data) {
    Int32 value;
    std::memcpy(&value, data, sizeof(Int32));
    // might need to convert network to host byte order
    data = static_cast<uint8_t*>(data) + sizeof(Int32);
    return value;
}

template<>
Int64 readData<Int64>(void*& data) {
    Int64 value;
    std::memcpy(&value, data, sizeof(Int64));
    // might need to convert network to host byte order
    data = static_cast<uint8_t*>(data) + sizeof(Int64);
    return value;
}