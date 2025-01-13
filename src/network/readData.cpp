#include <Standards.hpp>
#include <cstring>
#include <cstdint> // for uint8_t
#include <Console.hpp>
#include <network/VarIntLong.hpp>

// More types added as packets are implemented

int readData(void*& data, Int8& dest, int size) {
    if (size != 0) {
        Console::getConsole().Error("Non zero size given for Int8 reading.");
        return -1;
    }
    std::memcpy(&dest, data, sizeof(Int8));
    data = static_cast<uint8_t*>(data) + sizeof(Int8);
    return 0;
}

int readData(void*& data, Int16& dest, int size) {
    if (size != 0) {
        Console::getConsole().Error("Non zero size given for Int16 reading.");
        return -1;
    }
    std::memcpy(&dest, data, sizeof(Int16));
    data = static_cast<uint8_t*>(data) + sizeof(Int16);
    return 0;
}

int readData(void*& data, Int32& dest, int size) {
    if (size != 0) {
        Console::getConsole().Error("Non zero size given for Int32 reading.");
        return -1;
    }
    std::memcpy(&dest, data, sizeof(Int32));
    data = static_cast<uint8_t*>(data) + sizeof(Int32);
    return 0;
}

int readData(void*& data, Int64& dest, int size) {
    if (size != 0) {
        Console::getConsole().Error("Non zero size given for Int64 reading.");
        return -1;
    }
    std::memcpy(&dest, data, sizeof(Int64));
    data = static_cast<uint8_t*>(data) + sizeof(Int64);
    return 0;
}

int readData(void*& data, UInt8& dest, int size) {
    if (size != 0) {
        Console::getConsole().Error("Non zero size given for UInt8 reading.");
        return -1;
    }
    std::memcpy(&dest, data, sizeof(UInt8));
    data = static_cast<uint8_t*>(data) + sizeof(UInt8);
    return 0;
}

int readData(void*& data, UInt16& dest, int size) {
    if (size != 0) {
        Console::getConsole().Error("Non zero size given for UInt16 reading.");
        return -1;
    }
    std::memcpy(&dest, data, sizeof(UInt16));
    data = static_cast<uint8_t*>(data) + sizeof(UInt16);
    return 0;
}

int readData(void*& data, UInt32& dest, int size) {
    if (size != 0) {
        Console::getConsole().Error("Non zero size given for UInt32 reading.");
        return -1;
    }
    std::memcpy(&dest, data, sizeof(UInt32));
    data = static_cast<uint8_t*>(data) + sizeof(UInt32);
    return 0;
}

int readData(void*& data, UInt64& dest, int size) {
    if (size != 0) {
        Console::getConsole().Error("Non zero size given for UInt64 reading.");
        return -1;
    }
    std::memcpy(&dest, data, sizeof(UInt64));
    data = static_cast<uint8_t*>(data) + sizeof(UInt64);
    return 0;
}

int readData(void*& data, string& dest, int size) {
    if (size == 0) {
        Console::getConsole().Error("Size not given to string read");
    }
    int strSize = VarInt(data).getValue();
    if (strSize != size) {
        Console::getConsole().Error("Size given does not match size gathered");
    }
    // memcpy correct number of bytes onto stack
    // convert UTF8 or UTF16 characters to string
    // iterate pointer
    #warning "Missing implementation"
    return 0;
}