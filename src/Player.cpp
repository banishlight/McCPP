#include <Player.hpp>
#include <EntityIdAllocator.hpp>
#include <Console.hpp>
#include <algorithm>

Player::Player() : _entityId(EntityIdAllocator::next()) {
    _uuid.resize(2);
}

string Player::getUsername() const {
    return _username;
}

void Player::setUsername(const string& username) {
    _username = username;
}

std::vector<long> Player::getUUID() const {
    return _uuid;
}

void Player::setUUID(std::vector<long> uuid) {
    if (uuid.size() != 2) {
        Console::getConsole().Error("Player::setUUID(): bad UUID vector size.");
        return;
    }
    _uuid = uuid;
}

int Player::getViewDistance() const {
    return _viewDistance;
}

void Player::setViewDistance(int viewDistance) {
    _viewDistance = viewDistance;
}

int Player::getEntityId() const {
    return _entityId;
}

int Player::getGamemode() const {
    return _gamemode;
}

void Player::setGamemode(int gamemode) {
    _gamemode = gamemode;
}

double Player::getX() const {
    return _x;
}

double Player::getY() const {
    return _y;
}

double Player::getZ() const {
    return _z;
}

float Player::getYaw() const {
    return _yaw;
}

float Player::getPitch() const {
    return _pitch;
}

void Player::setPosition(double x, double y, double z) {
    _x = x;
    _y = y;
    _z = z;
}

void Player::setRotation(float yaw, float pitch) {
    _yaw = yaw;
    _pitch = pitch;
}

bool Player::hasChunkLoaded(int chunkX, int chunkZ) const {
    return _loadedChunks.count({chunkX, chunkZ}) > 0;
}

void Player::markChunkLoaded(int chunkX, int chunkZ) {
    _loadedChunks.insert({chunkX, chunkZ});
}

void Player::markChunkUnloaded(int chunkX, int chunkZ) {
    _loadedChunks.erase({chunkX, chunkZ});
}

const std::set<std::pair<int, int>>& Player::getLoadedChunks() const {
    return _loadedChunks;
}

int Player::getCenterChunkX() const {
    return _centerChunkX;
}

int Player::getCenterChunkZ() const {
    return _centerChunkZ;
}

void Player::setCenterChunk(int chunkX, int chunkZ) {
    _centerChunkX = chunkX;
    _centerChunkZ = chunkZ;
}

const std::array<HotbarSlot, Player::HOTBAR_SIZE>& Player::getHotbar() const {
    return _hotbar;
}

void Player::setHotbarSlot(int index, Int32 itemId, Int32 count) {
    if (index < 0 || index >= HOTBAR_SIZE) {
        Console::getConsole().Error("Player::setHotbarSlot(): index out of range.");
        return;
    }
    _hotbar[index] = {itemId, count};
}

int Player::getSelectedSlot() const {
    return _selectedSlot;
}

void Player::setSelectedSlot(int slot) {
    if (slot < 0 || slot >= HOTBAR_SIZE) {
        Console::getConsole().Error("Player::setSelectedSlot(): slot out of range.");
        return;
    }
    _selectedSlot = slot;
}

// Every currently-mapped item (stone, dirt, grass_block) caps at vanilla's
// standard 64 -- no per-item stack size table exists yet (see ItemBlockMapping.hpp).
static constexpr Int32 MAX_STACK_SIZE = 64;

bool Player::hasRoomFor(Int32 itemId) const {
    for (const HotbarSlot& slot : _hotbar) {
        if (slot.itemId == -1) return true;
        if (slot.itemId == itemId && slot.count < MAX_STACK_SIZE) return true;
    }
    return false;
}

Int32 Player::addItemToHotbar(Int32 itemId, Int32 count, std::vector<int>& changedSlots) {
    for (int i = 0; i < HOTBAR_SIZE && count > 0; i++) {
        if (_hotbar[i].itemId != itemId) continue;
        Int32 room = MAX_STACK_SIZE - _hotbar[i].count;
        if (room <= 0) continue;
        Int32 added = std::min(room, count);
        _hotbar[i].count += added;
        count -= added;
        changedSlots.push_back(i);
    }
    for (int i = 0; i < HOTBAR_SIZE && count > 0; i++) {
        if (_hotbar[i].itemId != -1) continue;
        Int32 added = std::min(MAX_STACK_SIZE, count);
        _hotbar[i] = {itemId, added};
        count -= added;
        changedSlots.push_back(i);
    }
    return count;
}