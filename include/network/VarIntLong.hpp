#pragma once
#include <Standards.hpp>
#include <vector>

class VarInt {
    public:
        // VarInt(int num);
        VarInt(Int32 num);
        VarInt(void* packet);
        Int32 getRawValue();
        int getValue();
        std::vector<Int8> getBytes();

    private:
        Int32 decode(void*& packet);
        Int32 value = 0;
        std::vector<Int8> bytes;
        static Byte constexpr SEGMENT_BITS = 0x7F;
        static Byte constexpr CONTINUE_BIT = 0x80;
};

class VarLong {
    public:
        // VarLong(long long num);
        VarLong(Int64 num);
        VarLong(void* packet);
        Int64 getRawValue();
        long long getValue();
        std::vector<Int8> getBytes();
        
    private:
        Int64 decode(void*& packet);
        Int64 value;
        std::vector<Int8> bytes;
        static Byte constexpr SEGMENT_BITS = 0x7FL;
        static Byte constexpr CONTINUE_BIT = 0x80L;
};
