#include <tests/TestFramework.hpp>
#include <network/PacketUtils.hpp>

// Expected byte sequences are the published Java Edition protocol's VarInt
// examples: 7 data bits per byte, little-endian groups, high bit set on every
// byte except the last, negative values always taking the full 5 bytes.

namespace {
    struct VarIntCase {
        int value;
        std::vector<Byte> bytes;
    };

    const std::vector<VarIntCase> VAR_INT_CASES = {
        {0,           {0x00}},
        {1,           {0x01}},
        {2,           {0x02}},
        {127,         {0x7f}},
        {128,         {0x80, 0x01}},
        {255,         {0xff, 0x01}},
        {25565,       {0xdd, 0xc7, 0x01}},
        {2097151,     {0xff, 0xff, 0x7f}},
        {2147483647,  {0xff, 0xff, 0xff, 0xff, 0x07}},
        {-1,          {0xff, 0xff, 0xff, 0xff, 0x0f}},
        {-2147483647 - 1, {0x80, 0x80, 0x80, 0x80, 0x08}},
    };
}

TEST(VarIntSerializeMatchesProtocolExamples) {
    for (const VarIntCase& c : VAR_INT_CASES) {
        CHECK_EQ(varIntSerialize(c.value), c.bytes);
    }
}

TEST(VarIntDeserializeMatchesProtocolExamples) {
    for (const VarIntCase& c : VAR_INT_CASES) {
        CHECK_EQ(varIntDeserialize(c.bytes), c.value);
    }
}

TEST(VarIntSizeMatchesEncodedLength) {
    for (const VarIntCase& c : VAR_INT_CASES) {
        CHECK_EQ(getVarIntSize(c.value), static_cast<int>(c.bytes.size()));
    }
}

TEST(DeserializeVarIntConsumesOnlyItsOwnBytes) {
    std::vector<Byte> data = {0xdd, 0xc7, 0x01, 0x2a};
    CHECK_EQ(deserializeVarInt(data), 25565);
    REQUIRE_EQ(data.size(), static_cast<size_t>(1));
    CHECK_EQ(data[0], static_cast<Byte>(0x2a));
}

TEST(StringRoundTrip) {
    for (const string& s : {string(""), string("minecraft:overworld"), string("h\xc3\xa9llo")}) {
        std::vector<Byte> data = serializeString(s);
        data.push_back(0x99); // trailing byte must be left untouched
        CHECK_EQ(deserializeString(data), s);
        REQUIRE_EQ(data.size(), static_cast<size_t>(1));
        CHECK_EQ(data[0], static_cast<Byte>(0x99));
    }
}
