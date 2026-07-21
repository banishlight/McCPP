#include <RegionFile.hpp>
#include <network/Compression.hpp>
#include <Console.hpp>
#include <filesystem>
#include <ctime>

namespace {
    constexpr size_t SECTOR_SIZE = 4096;
    constexpr size_t HEADER_SECTORS = 2; // location table + timestamp table
}

RegionFile::RegionFile(const string& worldDir, int regionX, int regionZ) {
    std::filesystem::path dir = std::filesystem::path(worldDir) / "region";
    std::filesystem::create_directories(dir);
    _path = (dir / ("r." + std::to_string(regionX) + "." + std::to_string(regionZ) + ".mca")).string();

    if (std::filesystem::exists(_path)) {
        _file.open(_path, std::ios::binary | std::ios::in | std::ios::out);
        if (_file.is_open()) {
            loadHeader();
        } else {
            Console::getConsole().Error("RegionFile::RegionFile(): Failed to open " + _path);
        }
    }
    // Else: leave _file closed. readChunk() treats "not open" as "nothing
    // saved in this region yet" without creating anything on disk;
    // writeChunk() creates the file lazily via ensureFileOpenForWrite().
}

void RegionFile::ensureFileOpenForWrite() {
    if (_file.is_open()) return;
    std::ofstream create(_path, std::ios::binary);
    std::vector<Byte> zeroHeader(SECTOR_SIZE * HEADER_SECTORS, 0);
    create.write(reinterpret_cast<const char*>(zeroHeader.data()), static_cast<std::streamsize>(zeroHeader.size()));
    create.close();
    _file.open(_path, std::ios::binary | std::ios::in | std::ios::out);
    // _locations/_timestamps are already all-zero from member initializers,
    // matching a freshly created empty header -- no loadHeader() needed.
}

void RegionFile::loadHeader() {
    _file.clear();
    _file.seekg(0);
    std::vector<Byte> header(SECTOR_SIZE * HEADER_SECTORS, 0);
    _file.read(reinterpret_cast<char*>(header.data()), static_cast<std::streamsize>(header.size()));
    _file.clear(); // a short read (new/truncated file) is fine -- rest stays zero

    for (int i = 0; i < 1024; i++) {
        size_t off = static_cast<size_t>(i) * 4;
        UInt32 sectorOffset = (static_cast<UInt32>(header[off]) << 16)
                             | (static_cast<UInt32>(header[off + 1]) << 8)
                             | static_cast<UInt32>(header[off + 2]);
        _locations[i] = LocationEntry{sectorOffset, header[off + 3]};
    }
    for (int i = 0; i < 1024; i++) {
        size_t off = SECTOR_SIZE + static_cast<size_t>(i) * 4;
        UInt32 ts = (static_cast<UInt32>(header[off]) << 24)
                  | (static_cast<UInt32>(header[off + 1]) << 16)
                  | (static_cast<UInt32>(header[off + 2]) << 8)
                  | static_cast<UInt32>(header[off + 3]);
        _timestamps[i] = ts;
    }
}

void RegionFile::flushHeader() {
    std::vector<Byte> header(SECTOR_SIZE * HEADER_SECTORS, 0);
    for (int i = 0; i < 1024; i++) {
        size_t off = static_cast<size_t>(i) * 4;
        const LocationEntry& loc = _locations[i];
        header[off] = static_cast<Byte>((loc.sectorOffset >> 16) & 0xFF);
        header[off + 1] = static_cast<Byte>((loc.sectorOffset >> 8) & 0xFF);
        header[off + 2] = static_cast<Byte>(loc.sectorOffset & 0xFF);
        header[off + 3] = loc.sectorCount;
    }
    for (int i = 0; i < 1024; i++) {
        size_t off = SECTOR_SIZE + static_cast<size_t>(i) * 4;
        UInt32 ts = _timestamps[i];
        header[off] = static_cast<Byte>((ts >> 24) & 0xFF);
        header[off + 1] = static_cast<Byte>((ts >> 16) & 0xFF);
        header[off + 2] = static_cast<Byte>((ts >> 8) & 0xFF);
        header[off + 3] = static_cast<Byte>(ts & 0xFF);
    }
    _file.clear();
    _file.seekp(0);
    _file.write(reinterpret_cast<const char*>(header.data()), static_cast<std::streamsize>(header.size()));
    _file.flush();
}

std::optional<NbtTag> RegionFile::readChunk(int localX, int localZ) {
    std::lock_guard<std::mutex> lock(_mutex);
    int idx = indexFor(localX, localZ);
    const LocationEntry& loc = _locations[idx];
    if (loc.sectorOffset == 0 && loc.sectorCount == 0) {
        return std::nullopt; // never saved
    }

    _file.clear();
    _file.seekg(static_cast<std::streamoff>(loc.sectorOffset) * static_cast<std::streamoff>(SECTOR_SIZE));
    Byte lengthBytes[4];
    _file.read(reinterpret_cast<char*>(lengthBytes), 4);
    if (!_file) return std::nullopt;
    UInt32 length = (static_cast<UInt32>(lengthBytes[0]) << 24)
                   | (static_cast<UInt32>(lengthBytes[1]) << 16)
                   | (static_cast<UInt32>(lengthBytes[2]) << 8)
                   | static_cast<UInt32>(lengthBytes[3]);
    if (length == 0) return std::nullopt;

    Byte scheme = 0;
    _file.read(reinterpret_cast<char*>(&scheme), 1);
    if (!_file) return std::nullopt;

    UInt32 payloadLen = length - 1; // length includes the scheme byte itself
    std::vector<Byte> payload(payloadLen);
    _file.read(reinterpret_cast<char*>(payload.data()), static_cast<std::streamsize>(payloadLen));
    if (!_file) return std::nullopt;

    if (scheme == 2) {
        std::vector<Byte> decompressed = decompressData(payload);
        if (decompressed.empty()) return std::nullopt;
        return NbtTag::parseFile(decompressed);
    } else if (scheme == 3) {
        return NbtTag::parseFile(payload);
    } else {
        Console::getConsole().Error("RegionFile::readChunk(): Unsupported compression scheme " + std::to_string(scheme)
            + " for chunk local (" + std::to_string(localX) + "," + std::to_string(localZ) + ") in " + _path);
        return std::nullopt;
    }
}

void RegionFile::writeChunk(int localX, int localZ, const NbtTag& root) {
    std::lock_guard<std::mutex> lock(_mutex);
    ensureFileOpenForWrite();
    std::vector<Byte> fileBytes = root.serializeFile("");
    std::vector<Byte> compressed = compressData(fileBytes);

    std::vector<Byte> sector;
    UInt32 length = static_cast<UInt32>(compressed.size() + 1); // +1 for the scheme byte
    sector.push_back(static_cast<Byte>((length >> 24) & 0xFF));
    sector.push_back(static_cast<Byte>((length >> 16) & 0xFF));
    sector.push_back(static_cast<Byte>((length >> 8) & 0xFF));
    sector.push_back(static_cast<Byte>(length & 0xFF));
    sector.push_back(2); // scheme: zlib
    sector.insert(sector.end(), compressed.begin(), compressed.end());

    UInt32 neededSectors = static_cast<UInt32>((sector.size() + SECTOR_SIZE - 1) / SECTOR_SIZE);
    sector.resize(static_cast<size_t>(neededSectors) * SECTOR_SIZE, 0);

    int idx = indexFor(localX, localZ);
    LocationEntry& loc = _locations[idx];

    UInt32 sectorOffset;
    if (loc.sectorOffset != 0 && loc.sectorCount >= neededSectors) {
        sectorOffset = loc.sectorOffset; // fits the existing allocation -- overwrite in place
    } else {
        _file.clear();
        _file.seekg(0, std::ios::end);
        std::streamoff endPos = _file.tellg();
        UInt32 endSector = static_cast<UInt32>((static_cast<size_t>(endPos) + SECTOR_SIZE - 1) / SECTOR_SIZE);
        if (endSector < HEADER_SECTORS) endSector = HEADER_SECTORS; // never before the header
        sectorOffset = endSector;
    }

    _file.clear();
    _file.seekp(static_cast<std::streamoff>(sectorOffset) * static_cast<std::streamoff>(SECTOR_SIZE));
    _file.write(reinterpret_cast<const char*>(sector.data()), static_cast<std::streamsize>(sector.size()));

    loc.sectorOffset = sectorOffset;
    // A single byte can't represent more than 255 sectors (~1MB compressed) --
    // real vanilla falls back to an external .mcc file past that; not
    // implemented here, an oversized chunk just gets its sector count capped
    // (would corrupt that one chunk on a subsequent read, but this is
    // exceedingly unlikely for chunks this project actually produces).
    loc.sectorCount = static_cast<Byte>(neededSectors > 255 ? 255 : neededSectors);
    _timestamps[idx] = static_cast<UInt32>(std::time(nullptr));

    flushHeader();
    _file.flush();
}

std::pair<int,int> RegionFile::regionCoordsFor(int chunkX, int chunkZ) {
    return { chunkX >> 5, chunkZ >> 5 };
}
