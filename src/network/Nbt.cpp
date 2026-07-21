#include <network/Nbt.hpp>
#include <Console.hpp>
#include <cstring>

NbtTag NbtTag::makeByte(Int8 value) {
    NbtTag t;
    t._type = NbtTagType::Byte;
    t._intValue = value;
    return t;
}

NbtTag NbtTag::makeShort(Int16 value) {
    NbtTag t;
    t._type = NbtTagType::Short;
    t._intValue = value;
    return t;
}

NbtTag NbtTag::makeInt(Int32 value) {
    NbtTag t;
    t._type = NbtTagType::Int;
    t._intValue = value;
    return t;
}

NbtTag NbtTag::makeLong(Int64 value) {
    NbtTag t;
    t._type = NbtTagType::Long;
    t._intValue = value;
    return t;
}

NbtTag NbtTag::makeFloat(float value) {
    NbtTag t;
    t._type = NbtTagType::Float;
    t._floatValue = value;
    return t;
}

NbtTag NbtTag::makeDouble(double value) {
    NbtTag t;
    t._type = NbtTagType::Double;
    t._floatValue = value;
    return t;
}

NbtTag NbtTag::makeString(const string& value) {
    NbtTag t;
    t._type = NbtTagType::String;
    t._stringValue = value;
    return t;
}

NbtTag NbtTag::makeList(NbtTagType elementType, std::vector<NbtTag> values) {
    NbtTag t;
    t._type = NbtTagType::List;
    t._listElementType = elementType;
    t._listValues = std::move(values);
    return t;
}

NbtTag NbtTag::makeByteArray(std::vector<Int8> values) {
    NbtTag t;
    t._type = NbtTagType::ByteArray;
    t._byteArrayValues = std::move(values);
    return t;
}

NbtTag NbtTag::makeIntArray(std::vector<Int32> values) {
    NbtTag t;
    t._type = NbtTagType::IntArray;
    t._intArrayValues = std::move(values);
    return t;
}

NbtTag NbtTag::makeLongArray(std::vector<Int64> values) {
    NbtTag t;
    t._type = NbtTagType::LongArray;
    t._longArrayValues = std::move(values);
    return t;
}

NbtTag NbtTag::makeCompound() {
    NbtTag t;
    t._type = NbtTagType::Compound;
    return t;
}

NbtTag NbtTag::fromJson(const nlohmann::json& value, const string& fieldName) {
    if (value.is_object()) {
        NbtTag compound = makeCompound();
        for (auto it = value.begin(); it != value.end(); ++it) {
            compound.put(it.key(), fromJson(it.value(), it.key()));
        }
        return compound;
    }
    if (value.is_array()) {
        if (value.empty()) {
            return makeList(NbtTagType::End, {});
        }
        std::vector<NbtTag> elements;
        elements.reserve(value.size());
        for (const auto& element : value) {
            elements.push_back(fromJson(element, fieldName));
        }
        NbtTagType elementType = elements.front()._type;
        return makeList(elementType, std::move(elements));
    }
    if (value.is_string()) {
        return makeString(value.get<string>());
    }
    if (value.is_boolean()) {
        return makeByte(value.get<bool>() ? 1 : 0);
    }
    if (value.is_number_float()) {
        // Only dimension_type's coordinate_scale is actually a Double; every
        // other fractional field vanilla uses (temperature, downfall, exhaustion,
        // ambient_light, ...) is a Float.
        if (fieldName == "coordinate_scale") {
            return makeDouble(value.get<double>());
        }
        return makeFloat(static_cast<float>(value.get<double>()));
    }
    if (value.is_number_integer()) {
        return makeInt(static_cast<Int32>(value.get<Int64>()));
    }
    // null or otherwise unhandled JSON value; shouldn't occur in vanilla registry data.
    return makeString("");
}

void NbtTag::put(const string& name, NbtTag value) {
    _children.emplace_back(name, std::move(value));
}

void NbtTag::writeNbtString(std::vector<Byte>& out, const string& value) {
    UInt16 len = static_cast<UInt16>(value.size());
    out.push_back(static_cast<Byte>((len >> 8) & 0xFF));
    out.push_back(static_cast<Byte>(len & 0xFF));
    out.insert(out.end(), value.begin(), value.end());
}

void NbtTag::serializePayload(std::vector<Byte>& out) const {
    switch (_type) {
        case NbtTagType::Byte:
            out.push_back(static_cast<Byte>(_intValue & 0xFF));
            break;
        case NbtTagType::Short:
            out.push_back(static_cast<Byte>((_intValue >> 8) & 0xFF));
            out.push_back(static_cast<Byte>(_intValue & 0xFF));
            break;
        case NbtTagType::Int:
            for (int i = 3; i >= 0; i--) {
                out.push_back(static_cast<Byte>((_intValue >> (i * 8)) & 0xFF));
            }
            break;
        case NbtTagType::Long:
            for (int i = 7; i >= 0; i--) {
                out.push_back(static_cast<Byte>((_intValue >> (i * 8)) & 0xFF));
            }
            break;
        case NbtTagType::Float: {
            float f = static_cast<float>(_floatValue);
            uint32_t bits;
            std::memcpy(&bits, &f, sizeof(bits));
            for (int i = 3; i >= 0; i--) {
                out.push_back(static_cast<Byte>((bits >> (i * 8)) & 0xFF));
            }
            break;
        }
        case NbtTagType::Double: {
            double d = _floatValue;
            uint64_t bits;
            std::memcpy(&bits, &d, sizeof(bits));
            for (int i = 7; i >= 0; i--) {
                out.push_back(static_cast<Byte>((bits >> (i * 8)) & 0xFF));
            }
            break;
        }
        case NbtTagType::String:
            writeNbtString(out, _stringValue);
            break;
        case NbtTagType::List: {
            out.push_back(static_cast<Byte>(_listElementType));
            Int32 count = static_cast<Int32>(_listValues.size());
            for (int i = 3; i >= 0; i--) {
                out.push_back(static_cast<Byte>((count >> (i * 8)) & 0xFF));
            }
            for (const auto& element : _listValues) {
                element.serializePayload(out);
            }
            break;
        }
        case NbtTagType::Compound:
            for (const auto& [name, child] : _children) {
                out.push_back(static_cast<Byte>(child._type));
                writeNbtString(out, name);
                child.serializePayload(out);
            }
            out.push_back(static_cast<Byte>(NbtTagType::End));
            break;
        case NbtTagType::LongArray: {
            Int32 count = static_cast<Int32>(_longArrayValues.size());
            for (int i = 3; i >= 0; i--) {
                out.push_back(static_cast<Byte>((count >> (i * 8)) & 0xFF));
            }
            for (Int64 value : _longArrayValues) {
                for (int i = 7; i >= 0; i--) {
                    out.push_back(static_cast<Byte>((value >> (i * 8)) & 0xFF));
                }
            }
            break;
        }
        case NbtTagType::ByteArray: {
            Int32 count = static_cast<Int32>(_byteArrayValues.size());
            for (int i = 3; i >= 0; i--) {
                out.push_back(static_cast<Byte>((count >> (i * 8)) & 0xFF));
            }
            for (Int8 value : _byteArrayValues) {
                out.push_back(static_cast<Byte>(value));
            }
            break;
        }
        case NbtTagType::IntArray: {
            Int32 count = static_cast<Int32>(_intArrayValues.size());
            for (int i = 3; i >= 0; i--) {
                out.push_back(static_cast<Byte>((count >> (i * 8)) & 0xFF));
            }
            for (Int32 value : _intArrayValues) {
                for (int i = 3; i >= 0; i--) {
                    out.push_back(static_cast<Byte>((value >> (i * 8)) & 0xFF));
                }
            }
            break;
        }
        case NbtTagType::End:
            break;
    }
}

std::vector<Byte> NbtTag::serializeNetwork() const {
    std::vector<Byte> out;
    out.push_back(static_cast<Byte>(_type));
    serializePayload(out);
    return out;
}

std::vector<Byte> NbtTag::serializeFile(const string& rootName) const {
    std::vector<Byte> out;
    out.push_back(static_cast<Byte>(_type));
    writeNbtString(out, rootName);
    serializePayload(out);
    return out;
}

const NbtTag* NbtTag::get(const string& name) const {
    for (const auto& [childName, child] : _children) {
        if (childName == name) return &child;
    }
    return nullptr;
}

Byte NbtTag::readU8(const std::vector<Byte>& data, size_t& cursor, bool& ok) {
    if (!ok || cursor + 1 > data.size()) { ok = false; return 0; }
    return data[cursor++];
}

Int16 NbtTag::readI16(const std::vector<Byte>& data, size_t& cursor, bool& ok) {
    if (!ok || cursor + 2 > data.size()) { ok = false; return 0; }
    UInt16 v = (static_cast<UInt16>(data[cursor]) << 8) | static_cast<UInt16>(data[cursor + 1]);
    cursor += 2;
    return static_cast<Int16>(v);
}

Int32 NbtTag::readI32(const std::vector<Byte>& data, size_t& cursor, bool& ok) {
    if (!ok || cursor + 4 > data.size()) { ok = false; return 0; }
    UInt32 v = 0;
    for (int i = 0; i < 4; i++) {
        v = (v << 8) | static_cast<UInt32>(data[cursor + i]);
    }
    cursor += 4;
    return static_cast<Int32>(v);
}

Int64 NbtTag::readI64(const std::vector<Byte>& data, size_t& cursor, bool& ok) {
    if (!ok || cursor + 8 > data.size()) { ok = false; return 0; }
    UInt64 v = 0;
    for (int i = 0; i < 8; i++) {
        v = (v << 8) | static_cast<UInt64>(data[cursor + i]);
    }
    cursor += 8;
    return static_cast<Int64>(v);
}

float NbtTag::readF32(const std::vector<Byte>& data, size_t& cursor, bool& ok) {
    Int32 bits = readI32(data, cursor, ok);
    float f;
    std::memcpy(&f, &bits, sizeof(f));
    return f;
}

double NbtTag::readF64(const std::vector<Byte>& data, size_t& cursor, bool& ok) {
    Int64 bits = readI64(data, cursor, ok);
    double d;
    std::memcpy(&d, &bits, sizeof(d));
    return d;
}

string NbtTag::readNbtString(const std::vector<Byte>& data, size_t& cursor, bool& ok) {
    UInt16 len = static_cast<UInt16>(readI16(data, cursor, ok));
    if (!ok || cursor + len > data.size()) { ok = false; return ""; }
    string s(reinterpret_cast<const char*>(data.data() + cursor), len);
    cursor += len;
    return s;
}

NbtTag NbtTag::parsePayload(const std::vector<Byte>& data, size_t& cursor, NbtTagType type, bool& ok) {
    switch (type) {
        case NbtTagType::Byte:
            return makeByte(static_cast<Int8>(readU8(data, cursor, ok)));
        case NbtTagType::Short:
            return makeShort(readI16(data, cursor, ok));
        case NbtTagType::Int:
            return makeInt(readI32(data, cursor, ok));
        case NbtTagType::Long:
            return makeLong(readI64(data, cursor, ok));
        case NbtTagType::Float:
            return makeFloat(readF32(data, cursor, ok));
        case NbtTagType::Double:
            return makeDouble(readF64(data, cursor, ok));
        case NbtTagType::String:
            return makeString(readNbtString(data, cursor, ok));
        case NbtTagType::ByteArray: {
            Int32 count = readI32(data, cursor, ok);
            std::vector<Int8> values;
            if (ok && count > 0) {
                values.reserve(static_cast<size_t>(count));
                for (Int32 i = 0; i < count; i++) {
                    values.push_back(static_cast<Int8>(readU8(data, cursor, ok)));
                }
            }
            return makeByteArray(std::move(values));
        }
        case NbtTagType::IntArray: {
            Int32 count = readI32(data, cursor, ok);
            std::vector<Int32> values;
            if (ok && count > 0) {
                values.reserve(static_cast<size_t>(count));
                for (Int32 i = 0; i < count; i++) {
                    values.push_back(readI32(data, cursor, ok));
                }
            }
            return makeIntArray(std::move(values));
        }
        case NbtTagType::LongArray: {
            Int32 count = readI32(data, cursor, ok);
            std::vector<Int64> values;
            if (ok && count > 0) {
                values.reserve(static_cast<size_t>(count));
                for (Int32 i = 0; i < count; i++) {
                    values.push_back(readI64(data, cursor, ok));
                }
            }
            return makeLongArray(std::move(values));
        }
        case NbtTagType::List: {
            NbtTagType elementType = static_cast<NbtTagType>(readU8(data, cursor, ok));
            Int32 count = readI32(data, cursor, ok);
            std::vector<NbtTag> values;
            if (ok && elementType != NbtTagType::End && count > 0) {
                values.reserve(static_cast<size_t>(count));
                for (Int32 i = 0; i < count && ok; i++) {
                    values.push_back(parsePayload(data, cursor, elementType, ok));
                }
            }
            return makeList(elementType, std::move(values));
        }
        case NbtTagType::Compound: {
            NbtTag compound = makeCompound();
            while (ok) {
                NbtTagType childType = static_cast<NbtTagType>(readU8(data, cursor, ok));
                if (!ok || childType == NbtTagType::End) break;
                string childName = readNbtString(data, cursor, ok);
                if (!ok) break;
                compound.put(childName, parsePayload(data, cursor, childType, ok));
            }
            return compound;
        }
        case NbtTagType::End:
            break;
    }
    return NbtTag();
}

NbtTag NbtTag::parseNamed(const std::vector<Byte>& data, size_t& cursor, string& outName, bool& ok) {
    NbtTagType type = static_cast<NbtTagType>(readU8(data, cursor, ok));
    if (!ok || type == NbtTagType::End) {
        outName = "";
        return NbtTag();
    }
    outName = readNbtString(data, cursor, ok);
    if (!ok) return NbtTag();
    return parsePayload(data, cursor, type, ok);
}

NbtTag NbtTag::parseFile(const std::vector<Byte>& data) {
    size_t cursor = 0;
    bool ok = true;
    string rootName;
    NbtTag result = parseNamed(data, cursor, rootName, ok);
    if (!ok) {
        Console::getConsole().Error("NbtTag::parseFile(): Malformed or truncated NBT data.");
        return NbtTag();
    }
    return result;
}