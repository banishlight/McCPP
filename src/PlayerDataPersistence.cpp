#include <PlayerDataPersistence.hpp>
#include <VanillaVersion.hpp>
#include <ItemNames.hpp>
#include <network/Nbt.hpp>
#include <network/Compression.hpp>
#include <network/PacketUtils.hpp>
#include <Console.hpp>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>

namespace {
    std::filesystem::path pathFor(const string& worldDir, const std::vector<long>& uuid) {
        return std::filesystem::path(worldDir) / "playerdata" / (uuidToDashedHexString(uuid) + ".dat");
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
            if (slot < 0 || slot >= Player::TOTAL_SLOTS) continue;
            result.inventory[slot] = {ItemNames::itemNameToId(idTag->asString()), countTag->asInt()};
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
    // slot numbering matches Player's own absolute inventory indices (see
    // Player.hpp's TOTAL_SLOTS/HOTBAR_START layout comment).
    std::vector<NbtTag> inventoryEntries;
    const std::array<InventorySlot, Player::TOTAL_SLOTS>& inventory = player.getInventory();
    for (int i = 0; i < Player::TOTAL_SLOTS; i++) {
        if (inventory[i].itemId == -1) continue;
        NbtTag entry = NbtTag::makeCompound();
        entry.put("Slot", NbtTag::makeByte(static_cast<Int8>(i)));
        entry.put("id", NbtTag::makeString(ItemNames::itemIdToName(inventory[i].itemId)));
        entry.put("Count", NbtTag::makeByte(static_cast<Int8>(inventory[i].count)));
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
