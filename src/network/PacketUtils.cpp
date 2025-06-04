#include <network/PacketUtils.hpp>
#include <Standards.hpp>
#include <vector>

std::vector<Byte> varIntSerialize(int num){ 
    std::vector<Byte> result;
    uint32_t value = static_cast<uint32_t>(num);
    while (true) {
        Byte temp = static_cast<Byte>(value & 0x7F);
        value >>= 7;
        
        if (value != 0) {
            temp |= 0x80;  // Set continuation bit
        }
        
        result.push_back(temp);
        
        if (value == 0) {
            break;
        }
    }
    return result;
}

int varIntDeserialize(std::vector<Byte> buff) {
    if (buff.empty()) {
        // throw std::runtime_error("Buffer is empty");
    }
    int32_t value = 0;
    int position = 0;
    for (size_t i = 0; i < buff.size(); ++i) {
        Byte currentByte = buff[i];
        
        // Extract the 7 data bits
        value |= (currentByte & 0x7F) << position;
        
        // Check if this is the last byte (continuation bit not set)
        if ((currentByte & 0x80) == 0) {
            return static_cast<int>(value);
        }
        
        position += 7;
        
        // Prevent infinite loop and overflow
        if (position >= 32) {
            // throw std::runtime_error("VarInt is too big");
        }
    }
    // If we get here, the VarInt was incomplete
    // throw std::runtime_error("Incomplete VarInt");
    return 0;
}

std::vector<Byte> varLongSerialize(long num) {
    std::vector<Byte> result;
    uint64_t value = static_cast<uint64_t>(num);
    while (true) {
        Byte temp = static_cast<Byte>(value & 0x7F);
        value >>= 7;
        
        if (value != 0) {
            temp |= 0x80;  // Set continuation bit
        }
        
        result.push_back(temp);
        
        if (value == 0) {
            break;
        }
    }
    return result;
}

long varLongDeserialize(std::vector<Byte> buff) {
    if (buff.empty()) {
        // throw std::runtime_error("Buffer is empty");
    }
    
    int64_t value = 0;
    int position = 0;
    
    for (size_t i = 0; i < buff.size(); ++i) {
        Byte currentByte = buff[i];
        
        // Extract the 7 data bits
        value |= (static_cast<int64_t>(currentByte & 0x7F)) << position;
        
        // Check if this is the last byte (continuation bit not set)
        if ((currentByte & 0x80) == 0) {
            return static_cast<long>(value);
        }
        
        position += 7;
        
        // Prevent infinite loop and overflow
        if (position >= 64) {
            // throw std::runtime_error("VarLong is too big");
        }
    }
    
    // If we get here, the VarLong was incomplete
    // throw std::runtime_error("Incomplete VarLong");
    return 0;
}

int getVarIntSize(int num) {
    uint32_t value = static_cast<uint32_t>(num);
    int size = 0;
    do {
        value >>= 7;
        size++;
    } while (value != 0);
    return size;
}

int getVarLongSize(long num) {
    uint64_t value = static_cast<uint64_t>(num);
    int size = 0;
    do {
        value >>= 7;
        size++;
    } while (value != 0);
    return size;
}

string deserializeString(std::vector<Byte>& data) {
    int size = varIntDeserialize(data);
    int varIntSize = getVarIntSize(size);
    string result(data.begin() + varIntSize, data.begin() + varIntSize + size);
    data.erase(data.begin(), data.begin() + varIntSize + size);
    return result;
}

std::vector<Byte> serializeString(const string& str) {
    std::vector<Byte> result = varIntSerialize(static_cast<int>(str.size()));
    result.insert(result.end(), str.begin(), str.end());
    return result;
}

std::vector<Byte> serializePrefixedArray(const std::vector<Byte>& data) {
    std::vector<Byte> result = varIntSerialize(data.size());
    result.insert(result.end(), data.begin(), data.begin());
    return result;
}
