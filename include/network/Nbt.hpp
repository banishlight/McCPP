#pragma once
#include <Standards.hpp>
#include <vector>
#include <utility>

enum class NbtTagType : Byte {
    End = 0x00,
    Byte = 0x01,
    Short = 0x02,
    Int = 0x03,
    Long = 0x04,
    Float = 0x05,
    Double = 0x06,
    String = 0x08,
    List = 0x09,
    Compound = 0x0A,
};

// Minimal NBT tag builder/serializer, covering just the tag types the
// Configuration/Play registries and chunk data need (no byte/int/long arrays yet).
class NbtTag {
    public:
        static NbtTag makeByte(Int8 value);
        static NbtTag makeShort(Int16 value);
        static NbtTag makeInt(Int32 value);
        static NbtTag makeLong(Int64 value);
        static NbtTag makeFloat(float value);
        static NbtTag makeDouble(double value);
        static NbtTag makeString(const string& value);
        static NbtTag makeList(NbtTagType elementType, std::vector<NbtTag> values);
        static NbtTag makeCompound();

        // Only valid when this tag is a Compound; appends a named child in insertion order.
        void put(const string& name, NbtTag value);

        // Serializes this tag as a "network" NBT value: the outer tag has no name
        // prefix, matching how the protocol embeds NBT since 1.20.2 (Registry Data
        // entries, chat components, etc.) as opposed to the file-based NBT format.
        std::vector<Byte> serializeNetwork() const;

    private:
        NbtTagType _type = NbtTagType::End;
        Int64 _intValue = 0;
        double _floatValue = 0;
        string _stringValue;
        std::vector<std::pair<string, NbtTag>> _children;
        NbtTagType _listElementType = NbtTagType::End;
        std::vector<NbtTag> _listValues;

        void serializePayload(std::vector<Byte>& out) const;
        static void writeNbtString(std::vector<Byte>& out, const string& value);
};