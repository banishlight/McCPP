#include <network/Nbt.hpp>
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