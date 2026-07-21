#pragma once
#include <Standards.hpp>
#include <vector>
#include <utility>
#include <lib/json.hpp>

enum class NbtTagType : Byte {
    End = 0x00,
    Byte = 0x01,
    Short = 0x02,
    Int = 0x03,
    Long = 0x04,
    Float = 0x05,
    Double = 0x06,
    ByteArray = 0x07,
    String = 0x08,
    List = 0x09,
    Compound = 0x0A,
    IntArray = 0x0B,
    LongArray = 0x0C,
};

// NBT tag builder/serializer/parser -- covers what the Configuration/Play
// registries, chunk data, and (as of world-save reading) region files and
// level.dat need.
class NbtTag {
    public:
        static NbtTag makeByte(Int8 value);
        static NbtTag makeShort(Int16 value);
        static NbtTag makeInt(Int32 value);
        static NbtTag makeLong(Int64 value);
        static NbtTag makeFloat(float value);
        static NbtTag makeDouble(double value);
        static NbtTag makeByteArray(std::vector<Int8> values);
        static NbtTag makeString(const string& value);
        static NbtTag makeList(NbtTagType elementType, std::vector<NbtTag> values);
        static NbtTag makeIntArray(std::vector<Int32> values);
        static NbtTag makeLongArray(std::vector<Int64> values);
        static NbtTag makeCompound();

        // Converts an arbitrary JSON value (as parsed by nlohmann::json, e.g. from a
        // vanilla registry file) into an NBT tag tree. JSON doesn't distinguish
        // Float/Double/Int/Long the way NBT requires, so this uses "has a decimal
        // point -> Float" as the default and takes a field-name hint for the rare
        // fields that are actually Double (currently just dimension_type's
        // coordinate_scale) or Long. Empty JSON arrays become an empty NBT List
        // tagged TAG_End, matching vanilla's own convention.
        static NbtTag fromJson(const nlohmann::json& value, const string& fieldName = "");

        // Only valid when this tag is a Compound; appends a named child in insertion order.
        void put(const string& name, NbtTag value);

        // Serializes this tag as a "network" NBT value: the outer tag has no name
        // prefix, matching how the protocol embeds NBT since 1.20.2 (Registry Data
        // entries, chat components, etc.) as opposed to the file-based NBT format.
        std::vector<Byte> serializeNetwork() const;

        // Serializes this tag as a *file*-format NBT value: unlike the network
        // form, the root tag carries a type byte AND a name (region files and
        // level.dat both use an empty-string root name in practice, but this
        // takes the name as a parameter rather than hardcoding it).
        std::vector<Byte> serializeFile(const string& rootName = "") const;

        // Parses a full named-root file-format buffer (the inverse of
        // serializeFile). Returns a default (End-typed) tag and logs an error
        // on any malformed input, rather than throwing -- callers (region file
        // reads, level.dat reads) are expected to treat that the same as
        // "chunk/file absent or corrupt" and fall back accordingly.
        static NbtTag parseFile(const std::vector<Byte>& data);

        // Read accessors. Callers are expected to check type() (or that get()
        // returned non-null) before calling the matching accessor -- these
        // don't themselves re-validate, matching how the write-side make*
        // functions don't validate their inputs either.
        NbtTagType type() const { return _type; }
        Int64 asLong() const { return _intValue; }
        Int32 asInt() const { return static_cast<Int32>(_intValue); }
        double asDouble() const { return _floatValue; }
        const string& asString() const { return _stringValue; }
        const std::vector<Int8>& asByteArray() const { return _byteArrayValues; }
        const std::vector<Int32>& asIntArray() const { return _intArrayValues; }
        const std::vector<Int64>& asLongArray() const { return _longArrayValues; }
        const std::vector<NbtTag>& asList() const { return _listValues; }
        // Only valid when this tag is a Compound; nullptr if no child has that name.
        const NbtTag* get(const string& name) const;

    private:
        NbtTagType _type = NbtTagType::End;
        Int64 _intValue = 0;
        double _floatValue = 0;
        string _stringValue;
        std::vector<std::pair<string, NbtTag>> _children;
        NbtTagType _listElementType = NbtTagType::End;
        std::vector<NbtTag> _listValues;
        std::vector<Int8> _byteArrayValues;
        std::vector<Int32> _intArrayValues;
        std::vector<Int64> _longArrayValues;

        void serializePayload(std::vector<Byte>& out) const;
        static void writeNbtString(std::vector<Byte>& out, const string& value);

        // Parsing helpers. `ok` is set false (and left false on every
        // subsequent call in the same parse chain) the moment anything reads
        // past the end of `data` -- callers just check it once at the end
        // rather than threading error handling through every recursive call.
        static Byte readU8(const std::vector<Byte>& data, size_t& cursor, bool& ok);
        static Int16 readI16(const std::vector<Byte>& data, size_t& cursor, bool& ok);
        static Int32 readI32(const std::vector<Byte>& data, size_t& cursor, bool& ok);
        static Int64 readI64(const std::vector<Byte>& data, size_t& cursor, bool& ok);
        static float readF32(const std::vector<Byte>& data, size_t& cursor, bool& ok);
        static double readF64(const std::vector<Byte>& data, size_t& cursor, bool& ok);
        static string readNbtString(const std::vector<Byte>& data, size_t& cursor, bool& ok);
        static NbtTag parsePayload(const std::vector<Byte>& data, size_t& cursor, NbtTagType type, bool& ok);
        static NbtTag parseNamed(const std::vector<Byte>& data, size_t& cursor, string& outName, bool& ok);
};

// A single registry entry (e.g. one biome, one dimension type) ready to hand
// to Registry_Data_p: its identifier, whether it carries data, and the NBT
// tag tree if so.
struct RegistryEntry {
    string id;
    bool hasData;
    NbtTag data; // only read if hasData is true
};