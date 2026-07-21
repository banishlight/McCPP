#include <PlayerDataPersistence.hpp>
#include <VanillaVersion.hpp>
#include <BlockNames.hpp>
#include <ItemBlockMapping.hpp>
#include <network/Nbt.hpp>
#include <network/Compression.hpp>
#include <network/PacketUtils.hpp>
#include <Console.hpp>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>

namespace {
    // Vanilla playerdata filenames are dashed (8-4-4-4-12); the only existing
    // UUID->string helper (uuidToHexString, network/PacketUtils.hpp) is
    // dashless (used today only as an OpsList lookup key) -- derive the
    // dashed form from it rather than reimplementing hex conversion.
    string dashedUuidString(const std::vector<long>& uuid) {
        string hex = uuidToHexString(uuid);
        if (hex.size() != 32) return hex; // shouldn't happen -- uuidToHexString always pads to 32
        return hex.substr(0, 8) + "-" + hex.substr(8, 4) + "-" + hex.substr(12, 4) + "-"
             + hex.substr(16, 4) + "-" + hex.substr(20, 12);
    }

    std::filesystem::path pathFor(const string& worldDir, const std::vector<long>& uuid) {
        return std::filesystem::path(worldDir) / "playerdata" / (dashedUuidString(uuid) + ".dat");
    }

    // Serializes all save() calls -- the first place in this codebase where
    // the *same* file could plausibly be written from two different threads
    // close together (a connection-pool thread's disconnect-save racing the
    // tick thread's periodic autosave-save for that same still-connected
    // player). A single mutex is simplest and sufficient given how infrequent
    // and cheap these writes are; no per-UUID lock map is needed.
    std::mutex g_saveMutex;
}

namespace PlayerDataPersistence {

std::optional<PlayerSaveData> tryLoad(const string& worldDir, const std::vector<long>& uuid) {
    std::filesystem::path path = pathFor(worldDir, uuid);
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return std::nullopt; // brand-new player -- not an error
    }

    std::vector<Byte> raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::vector<Byte> decompressed = decompressGzip(raw);
    if (decompressed.empty()) {
        Console::getConsole().Error("PlayerDataPersistence::tryLoad(): Failed to gzip-decompress " + path.string());
        return std::nullopt;
    }

    NbtTag root = NbtTag::parseFile(decompressed);
    PlayerSaveData result;

    if (const NbtTag* pos = root.get("Pos")) {
        const std::vector<NbtTag>& values = pos->asList();
        if (values.size() == 3) {
            result.x = values[0].asDouble();
            result.y = values[1].asDouble();
            result.z = values[2].asDouble();
        }
    }
    if (const NbtTag* rot = root.get("Rotation")) {
        const std::vector<NbtTag>& values = rot->asList();
        if (values.size() == 2) {
            result.yaw = static_cast<float>(values[0].asDouble());
            result.pitch = static_cast<float>(values[1].asDouble());
        }
    }
    if (const NbtTag* t = root.get("playerGameType")) result.gamemode = t->asInt();
    if (const NbtTag* t = root.get("SelectedItemSlot")) result.selectedSlot = t->asInt();

    if (const NbtTag* inventory = root.get("Inventory")) {
        for (const NbtTag& entry : inventory->asList()) {
            const NbtTag* slotTag = entry.get("Slot");
            const NbtTag* idTag = entry.get("id");
            const NbtTag* countTag = entry.get("Count");
            if (!slotTag || !idTag || !countTag) continue;
            int slot = slotTag->asInt();
            if (slot < 0 || slot >= Player::HOTBAR_SIZE) continue; // this project only has a hotbar, not a full 36-slot inventory
            Int32 blockStateId = BlockNames::blockNameToStateId(idTag->asString());
            result.hotbar[slot] = {blockStateIdToItemId(blockStateId), countTag->asInt()};
        }
    }

    return result;
}

void save(const string& worldDir, const Player& player) {
    NbtTag root = NbtTag::makeCompound();
    root.put("DataVersion", NbtTag::makeInt(VanillaVersion::CURRENT_DATA_VERSION));
    root.put("Pos", NbtTag::makeList(NbtTagType::Double, {
        NbtTag::makeDouble(player.getX()),
        NbtTag::makeDouble(player.getY()),
        NbtTag::makeDouble(player.getZ())
    }));
    root.put("Rotation", NbtTag::makeList(NbtTagType::Float, {
        NbtTag::makeFloat(player.getYaw()),
        NbtTag::makeFloat(player.getPitch())
    }));
    root.put("playerGameType", NbtTag::makeInt(player.getGamemode()));
    root.put("SelectedItemSlot", NbtTag::makeInt(player.getSelectedSlot()));

    // Only occupied slots are listed, matching vanilla's own convention --
    // hotbar slots map directly onto vanilla's own slot numbering (0-8 are
    // the hotbar in a full 36-slot inventory too).
    std::vector<NbtTag> inventoryEntries;
    const std::array<HotbarSlot, Player::HOTBAR_SIZE>& hotbar = player.getHotbar();
    for (int i = 0; i < Player::HOTBAR_SIZE; i++) {
        if (hotbar[i].itemId == -1) continue;
        NbtTag entry = NbtTag::makeCompound();
        entry.put("Slot", NbtTag::makeByte(static_cast<Int8>(i)));
        entry.put("id", NbtTag::makeString(BlockNames::blockStateIdToName(itemIdToBlockStateId(hotbar[i].itemId))));
        entry.put("Count", NbtTag::makeByte(static_cast<Int8>(hotbar[i].count)));
        inventoryEntries.push_back(entry);
    }
    root.put("Inventory", NbtTag::makeList(NbtTagType::Compound, inventoryEntries));

    std::vector<Byte> fileBytes = root.serializeFile("");
    std::vector<Byte> gzipped = compressGzip(fileBytes);

    std::filesystem::path finalPath = pathFor(worldDir, player.getUUID());
    std::filesystem::path tempPath = finalPath;
    tempPath += ".tmp";

    std::lock_guard<std::mutex> lock(g_saveMutex);
    std::filesystem::create_directories(finalPath.parent_path());
    {
        std::ofstream out(tempPath, std::ios::binary);
        out.write(reinterpret_cast<const char*>(gzipped.data()), static_cast<std::streamsize>(gzipped.size()));
    }
    std::error_code ec;
    std::filesystem::rename(tempPath, finalPath, ec); // same-filesystem rename is atomic -- crash-safety
    if (ec) {
        Console::getConsole().Error("PlayerDataPersistence::save(): Failed to rename " + tempPath.string() + " to " + finalPath.string() + ": " + ec.message());
    }
}

}
