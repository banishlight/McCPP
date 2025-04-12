#pragma once
#include <vector>
#include <cstdint>

class VarInt {
    public:
        VarInt();
        VarInt(int32_t num);
        VarInt(void* packet);
        int32_t getRawValue();
        int getValue();
        std::vector<uint8_t> getBytes() const;

    private:
        void encode();
        int32_t decode(void*& packet);
        int32_t value = 0;
        std::vector<uint8_t> bytes;
        static uint8_t constexpr SEGMENT_BITS = 0x7F;
        static uint8_t constexpr CONTINUE_BIT = 0x80;
};

class VarLong {
    public:
        VarLong();
        VarLong(int64_t num);
        VarLong(void* packet);
        int64_t getRawValue();
        long long getValue();
        std::vector<uint8_t> getBytes() const;
        
    private:
        void encode();
        int64_t decode(void*& packet);
        int64_t value;
        std::vector<uint8_t> bytes;
        static uint8_t constexpr SEGMENT_BITS = 0x7FL;
        static uint8_t constexpr CONTINUE_BIT = 0x80L;
};
