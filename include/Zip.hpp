#pragma once
#include <Standards.hpp>
#include <vector>
#include <map>

// Minimal read-only ZIP archive reader: central directory + local file header
// parsing, supporting the "stored" and "deflate" methods jar files use. No
// Zip64 support (not needed for jar-sized archives), no encryption, no
// multi-disk archives.
class ZipArchive {
    public:
        bool load(std::vector<Byte> data);
        bool hasEntry(const string& name) const;
        // Returns the decompressed bytes of a named entry, or empty on failure/missing.
        std::vector<Byte> extract(const string& name) const;
        // Entry names starting with prefix, in central-directory order.
        std::vector<string> listEntries(const string& prefix) const;
    private:
        struct CentralDirEntry {
            UInt16 compressionMethod;
            UInt32 compressedSize;
            UInt32 uncompressedSize;
            UInt32 localHeaderOffset;
        };
        std::vector<Byte> _data;
        std::map<string, CentralDirEntry> _entries;
        bool parseCentralDirectory();
};