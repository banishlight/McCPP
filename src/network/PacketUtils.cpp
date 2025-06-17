#include <network/PacketUtils.hpp>
#include <network/Compression.hpp>
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

std::vector<Byte> deserializePrefixedArray(std::vector<Byte>& data) {
    int size = varIntDeserialize(data);
    int varIntSize = getVarIntSize(size);
    if (data.size() < static_cast<size_t>(varIntSize + size)) {
        // Console::getConsole().Error("Not enough bytes to deserialize prefixed array");
        return {}; // Return empty vector
    }
    std::vector<Byte> result(data.begin() + varIntSize, data.begin() + varIntSize + size);
    data.erase(data.begin(), data.begin() + varIntSize + size);
    return result;
}

std::vector<Byte> serializePrefixedArray(const std::vector<Byte>& data) {
    std::vector<Byte> result = varIntSerialize(data.size());
    result.insert(result.end(), data.begin(), data.begin());
    return result;
}

std::vector<long> deserializeUUID(std::vector<Byte>& data) {
    std::vector<long> result;
    if (data.size() < 16) {
        // Console::getConsole().Error("Not enough bytes to deserialize UUID");
        return result; // Return empty vector
    }
    // Extract most significant 64 bits (first 8 bytes)
    long mostSignificant = 0;
    for (int i = 0; i < 8; i++) {
        mostSignificant = (mostSignificant << 8) | static_cast<long>(data[i]);
    }
    // Extract least significant 64 bits (next 8 bytes)
    long leastSignificant = 0;
    for (int i = 8; i < 16; i++) {
        leastSignificant = (leastSignificant << 8) | static_cast<long>(data[i]);
    }
    result.push_back(mostSignificant);
    result.push_back(leastSignificant);
    data.erase(data.begin(), data.begin() + 16);
    return result;
}

std::vector<Byte> serializeUUID(const std::vector<long>& data) {
    std::vector<Byte> result;
    if (data.size() < 16) {
        // Console::getConsole().Error("Not enough bytes to deserialize UUID");
        return result; // Return empty vector
    }
    // UUID is stored as two 64-bit values in big-endian format
    long mostSignificant = data[0];
    long leastSignificant = data[1];
    // Serialize most significant 64 bits (8 bytes, big-endian)
    for (int i = 7; i >= 0; i--) {
        result.push_back(static_cast<Byte>((mostSignificant >> (i * 8)) & 0xFF));
    }
    // Serialize least significant 64 bits (8 bytes, big-endian)
    for (int i = 7; i >= 0; i--) {
        result.push_back(static_cast<Byte>((leastSignificant >> (i * 8)) & 0xFF));
    }
    return result;
}

std::vector<Byte> assemblePacket(int id, int threshold, const std::vector<Byte>& data) {
    std::vector<Byte> packet;
    if (threshold < 0) {
        // No compression
        int size = data.size() + getVarIntSize(id);
        packet = varIntSerialize(size);
        std::vector<Byte> packetID = varIntSerialize(id);
        packet.insert(packet.end(), packetID.begin(), packetID.end());
        packet.insert(packet.end(), data.begin(), data.end());
    }
    else {
        // Compression enabled
        // 1. Packet length
        // 2. Data length
        // 3. Packet ID
        // 4. Data
        if (data.size() >= static_cast<unsigned long>(threshold)) {
            // Compress the data
            int size;
            std::vector<Byte> packet_id = varIntSerialize(id);
            std::vector<Byte> data_length = varIntSerialize(static_cast<int>(data.size() + packet_id.size()));
            std::vector<Byte> precomp_data = packet_id;
            precomp_data.insert(precomp_data.end(), data.begin(), data.end());
            std::vector<Byte> compressed_data = compressData(precomp_data);
            size = compressed_data.size() + data_length.size();
            packet = varIntSerialize(size);
            packet.insert(packet.end(), data_length.begin(), data_length.end());
            packet.insert(packet.end(), compressed_data.begin(), compressed_data.end());
        } else {
            // No compression, below threshold
            std::vector<Byte> data_length = varIntSerialize(0);
            std::vector<Byte> packetID = varIntSerialize(id);
            int size = data.size() + data_length.size() + packetID.size();
            packet = varIntSerialize(size);
            packet.insert(packet.end(), data_length.begin(), data_length.end());
            packet.insert(packet.end(), packetID.begin(), packetID.end());
            packet.insert(packet.end(), data.begin(), data.end());
        }
    }
    return packet;
}