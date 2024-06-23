#pragma once
#include <exception>
#include <vector>
#include "../Standards.hpp"

class VarInt
{
public:
    bool inUse;
    UInt32 varIntSize;

    Byte varIntBuf[5];
    Int32 ReadVarInt(Int32* sock, Int32* readVal);

    void WriteVarInt(Int32 value, Int32* sock, Int32* writeVal);

    UInt32 EncodedLength(UInt32 index);

private:
    static Byte constexpr SEGMENT_BITS = 0x7F;
    static Byte constexpr CONTINUE_BIT = 0x80;

};

class VarLong
{
public:
    UInt32 varIntSize;
    bool inUse;
    Byte varIntBuf[10];
    Int64 ReadVarLong();
    void WriteVarLong(Int64 value);
    
private:


    static Byte constexpr SEGMENT_BITS = 0x7FL;
    static Byte constexpr CONTINUE_BIT = 0x80L;

};
