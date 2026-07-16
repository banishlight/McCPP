#include <Zip.hpp>
#include <Console.hpp>
#include <cstring>
#include <zlib.h>

namespace {
    const Byte EOCD_SIG[4] = {0x50, 0x4B, 0x05, 0x06};
    const Byte CD_SIG[4] = {0x50, 0x4B, 0x01, 0x02};
    const Byte LOCAL_SIG[4] = {0x50, 0x4B, 0x03, 0x04};

    UInt16 readU16(const Byte* p) {
        return static_cast<UInt16>(p[0]) | (static_cast<UInt16>(p[1]) << 8);
    }

    UInt32 readU32(const Byte* p) {
        return static_cast<UInt32>(p[0]) | (static_cast<UInt32>(p[1]) << 8) |
               (static_cast<UInt32>(p[2]) << 16) | (static_cast<UInt32>(p[3]) << 24);
    }

    // ZIP entries use raw deflate (RFC 1951, no zlib header/trailer), unlike the
    // zlib-wrapped stream Compression.cpp handles for packet compression.
    std::vector<Byte> inflateRawDeflate(const std::vector<Byte>& compressed, UInt32 expectedSize) {
        if (expectedSize == 0) {
            return std::vector<Byte>();
        }
        std::vector<Byte> out(expectedSize);
        z_stream stream{};
        stream.next_in = const_cast<Bytef*>(compressed.data());
        stream.avail_in = static_cast<uInt>(compressed.size());
        stream.next_out = out.data();
        stream.avail_out = static_cast<uInt>(out.size());

        if (inflateInit2(&stream, -MAX_WBITS) != Z_OK) {
            Console::getConsole().Error("ZipArchive: Failed to initialize raw deflate decompression");
            return std::vector<Byte>();
        }
        int ret = inflate(&stream, Z_FINISH);
        inflateEnd(&stream);
        if (ret != Z_STREAM_END) {
            Console::getConsole().Error("ZipArchive: Raw deflate decompression failed");
            return std::vector<Byte>();
        }
        return out;
    }
}

bool ZipArchive::load(std::vector<Byte> data) {
    _data = std::move(data);
    _entries.clear();
    return parseCentralDirectory();
}

bool ZipArchive::parseCentralDirectory() {
    if (_data.size() < 22) {
        return false;
    }

    // The End of Central Directory record sits at the end of the file, after an
    // optional variable-length comment (max 65535 bytes), so scan backward for it.
    size_t searchFloor = (_data.size() >= 22 + 65536) ? _data.size() - 22 - 65536 : 0;
    long eocdPos = -1;
    for (long i = static_cast<long>(_data.size()) - 22; i >= static_cast<long>(searchFloor); i--) {
        if (std::memcmp(&_data[i], EOCD_SIG, 4) == 0) {
            eocdPos = i;
            break;
        }
    }
    if (eocdPos < 0) {
        Console::getConsole().Error("ZipArchive: Could not find End of Central Directory record");
        return false;
    }

    UInt32 cdOffset = readU32(&_data[eocdPos + 16]);
    UInt16 totalEntries = readU16(&_data[eocdPos + 10]);

    size_t pos = cdOffset;
    for (UInt16 i = 0; i < totalEntries; i++) {
        if (pos + 46 > _data.size() || std::memcmp(&_data[pos], CD_SIG, 4) != 0) {
            Console::getConsole().Error("ZipArchive: Malformed central directory entry");
            return false;
        }

        UInt16 compressionMethod = readU16(&_data[pos + 10]);
        UInt32 compressedSize = readU32(&_data[pos + 20]);
        UInt32 uncompressedSize = readU32(&_data[pos + 24]);
        UInt16 nameLen = readU16(&_data[pos + 28]);
        UInt16 extraLen = readU16(&_data[pos + 30]);
        UInt16 commentLen = readU16(&_data[pos + 32]);
        UInt32 localHeaderOffset = readU32(&_data[pos + 42]);

        string name(reinterpret_cast<const char*>(&_data[pos + 46]), nameLen);

        CentralDirEntry entry;
        entry.compressionMethod = compressionMethod;
        entry.compressedSize = compressedSize;
        entry.uncompressedSize = uncompressedSize;
        entry.localHeaderOffset = localHeaderOffset;
        _entries[name] = entry;

        pos += 46 + nameLen + extraLen + commentLen;
    }
    return true;
}

bool ZipArchive::hasEntry(const string& name) const {
    return _entries.find(name) != _entries.end();
}

std::vector<Byte> ZipArchive::extract(const string& name) const {
    auto it = _entries.find(name);
    if (it == _entries.end()) {
        return std::vector<Byte>();
    }
    const CentralDirEntry& entry = it->second;

    size_t pos = entry.localHeaderOffset;
    if (pos + 30 > _data.size() || std::memcmp(&_data[pos], LOCAL_SIG, 4) != 0) {
        Console::getConsole().Error("ZipArchive: Malformed local file header for " + name);
        return std::vector<Byte>();
    }

    UInt16 nameLen = readU16(&_data[pos + 26]);
    UInt16 extraLen = readU16(&_data[pos + 28]);
    size_t dataStart = pos + 30 + nameLen + extraLen;
    if (dataStart + entry.compressedSize > _data.size()) {
        Console::getConsole().Error("ZipArchive: Entry data out of bounds for " + name);
        return std::vector<Byte>();
    }

    std::vector<Byte> compressed(_data.begin() + dataStart, _data.begin() + dataStart + entry.compressedSize);

    if (entry.compressionMethod == 0) {
        return compressed;
    } else if (entry.compressionMethod == 8) {
        return inflateRawDeflate(compressed, entry.uncompressedSize);
    }
    Console::getConsole().Error("ZipArchive: Unsupported compression method for " + name);
    return std::vector<Byte>();
}

std::vector<string> ZipArchive::listEntries(const string& prefix) const {
    std::vector<string> result;
    for (const auto& [name, entry] : _entries) {
        (void)entry;
        if (name.compare(0, prefix.size(), prefix) == 0) {
            result.push_back(name);
        }
    }
    return result;
}