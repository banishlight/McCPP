#include <LevelDat.hpp>
#include <VanillaVersion.hpp>
#include <network/Nbt.hpp>
#include <network/Compression.hpp>
#include <Console.hpp>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace LevelDat {

std::optional<LevelData> tryLoad(const string& worldDir) {
    std::filesystem::path path = std::filesystem::path(worldDir) / "level.dat";
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return std::nullopt; // no existing world here -- not an error
    }

    std::vector<Byte> raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::vector<Byte> decompressed = decompressGzip(raw);
    if (decompressed.empty()) {
        Console::getConsole().Error("LevelDat::tryLoad(): Failed to gzip-decompress " + path.string());
        return std::nullopt;
    }

    NbtTag root = NbtTag::parseFile(decompressed);
    const NbtTag* data = root.get("Data");
    if (!data) {
        Console::getConsole().Error("LevelDat::tryLoad(): No 'Data' compound in " + path.string());
        return std::nullopt;
    }

    Int32 dataVersion = 0;
    if (const NbtTag* t = data->get("DataVersion")) dataVersion = t->asInt();
    if (dataVersion < VanillaVersion::MIN_SUPPORTED_DATA_VERSION) {
        Console::getConsole().Error("LevelDat::tryLoad(): Unsupported world version (DataVersion "
            + std::to_string(dataVersion) + ", need at least " + std::to_string(VanillaVersion::MIN_SUPPORTED_DATA_VERSION)
            + " / Java Edition 1.18) in " + path.string());
        return std::nullopt;
    }

    LevelData result;
    result.dataVersion = dataVersion;
    if (const NbtTag* t = data->get("RandomSeed")) result.seed = t->asLong();
    if (const NbtTag* t = data->get("LevelName")) result.levelName = t->asString();
    if (const NbtTag* t = data->get("SpawnX")) result.spawnX = static_cast<double>(t->asInt());
    if (const NbtTag* t = data->get("SpawnY")) result.spawnY = static_cast<double>(t->asInt());
    if (const NbtTag* t = data->get("SpawnZ")) result.spawnZ = static_cast<double>(t->asInt());
    if (const NbtTag* t = data->get("SpawnAngle")) result.spawnYaw = static_cast<float>(t->asDouble());
    if (const NbtTag* t = data->get("GameType")) result.gameType = t->asInt();
    if (const NbtTag* t = data->get("Difficulty")) result.difficulty = static_cast<int>(t->asLong());
    if (const NbtTag* t = data->get("hardcore")) result.hardcore = (t->asLong() != 0);
    if (const NbtTag* t = data->get("DayTime")) result.dayTime = t->asLong();

    return result;
}

void save(const string& worldDir, const LevelData& data) {
    std::filesystem::create_directories(worldDir);

    NbtTag dataCompound = NbtTag::makeCompound();
    dataCompound.put("DataVersion", NbtTag::makeInt(data.dataVersion));
    dataCompound.put("RandomSeed", NbtTag::makeLong(data.seed));
    dataCompound.put("LevelName", NbtTag::makeString(data.levelName));
    dataCompound.put("SpawnX", NbtTag::makeInt(static_cast<Int32>(data.spawnX)));
    dataCompound.put("SpawnY", NbtTag::makeInt(static_cast<Int32>(data.spawnY)));
    dataCompound.put("SpawnZ", NbtTag::makeInt(static_cast<Int32>(data.spawnZ)));
    dataCompound.put("SpawnAngle", NbtTag::makeFloat(data.spawnYaw));
    dataCompound.put("GameType", NbtTag::makeInt(data.gameType));
    dataCompound.put("Difficulty", NbtTag::makeByte(static_cast<Int8>(data.difficulty)));
    dataCompound.put("hardcore", NbtTag::makeByte(data.hardcore ? 1 : 0));
    dataCompound.put("DayTime", NbtTag::makeLong(data.dayTime));
    dataCompound.put("Time", NbtTag::makeLong(data.dayTime)); // no day/night cycle system yet -- doesn't advance independently

    NbtTag root = NbtTag::makeCompound();
    root.put("Data", dataCompound);

    std::vector<Byte> fileBytes = root.serializeFile("");
    std::vector<Byte> gzipped = compressGzip(fileBytes);

    std::filesystem::path finalPath = std::filesystem::path(worldDir) / "level.dat";
    std::filesystem::path tempPath = std::filesystem::path(worldDir) / "level.dat.tmp";
    {
        std::ofstream out(tempPath, std::ios::binary);
        out.write(reinterpret_cast<const char*>(gzipped.data()), static_cast<std::streamsize>(gzipped.size()));
    }
    std::error_code ec;
    std::filesystem::rename(tempPath, finalPath, ec); // same-filesystem rename is atomic -- crash-safety
    if (ec) {
        Console::getConsole().Error("LevelDat::save(): Failed to rename " + tempPath.string() + " to " + finalPath.string() + ": " + ec.message());
    }
}

}
