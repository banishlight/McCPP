#include <Player.hpp>
#include <EntityIdAllocator.hpp>
#include <ItemProperties.hpp>
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

const std::vector<PlayerProfileProperty>& Player::getProfileProperties() const {
    return _profileProperties;
}

void Player::setProfileProperties(std::vector<PlayerProfileProperty> properties) {
    _profileProperties = std::move(properties);
}

Byte Player::getSkinParts() const {
    return _skinParts;
}

void Player::setSkinParts(Byte skinParts) {
    _skinParts = skinParts;
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

bool Player::isSneaking() const {
    return _sneaking;
}

void Player::setSneaking(bool sneaking) {
    _sneaking = sneaking;
}

bool Player::isSprinting() const {
    return _sprinting;
}

void Player::setSprinting(bool sprinting) {
    _sprinting = sprinting;
}

bool Player::isInWater() const {
    return _inWater;
}

void Player::setInWater(bool inWater) {
    _inWater = inWater;
}

int Player::getPose() const {
    // Pose enum values confirmed against minecraft.wiki's Entity Metadata
    // page: STANDING=0, SWIMMING=3, SNEAKING=5. This ordinal list itself is
    // stable -- only the metadata TYPE id (see Set_Player_Pose_Metadata_p)
    // is version-fragile, not these values.
    if (_inWater) return 3;
    if (_sneaking) return 5;
    return 0;
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

const std::array<InventorySlot, Player::TOTAL_SLOTS>& Player::getInventory() const {
    return _slots;
}

const InventorySlot& Player::getSlot(int index) const {
    static const InventorySlot EMPTY{};
    if (index < 0 || index >= TOTAL_SLOTS) {
        Console::getConsole().Error("Player::getSlot(): index out of range.");
        return EMPTY;
    }
    return _slots[index];
}

void Player::setSlot(int index, Int32 itemId, Int32 count) {
    if (index < 0 || index >= TOTAL_SLOTS) {
        Console::getConsole().Error("Player::setSlot(): index out of range.");
        return;
    }
    _slots[index] = {itemId, count};
}

std::array<InventorySlot, Player::HOTBAR_SIZE> Player::getHotbar() const {
    std::array<InventorySlot, HOTBAR_SIZE> out;
    std::copy(_slots.begin() + HOTBAR_START, _slots.begin() + HOTBAR_START + HOTBAR_SIZE, out.begin());
    return out;
}

void Player::setHotbarSlot(int index, Int32 itemId, Int32 count) {
    if (index < 0 || index >= HOTBAR_SIZE) {
        Console::getConsole().Error("Player::setHotbarSlot(): index out of range.");
        return;
    }
    _slots[HOTBAR_START + index] = {itemId, count};
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

bool Player::hasRoomFor(Int32 itemId) const {
    Int32 maxStack = ItemProperties::getMaxStackSize(itemId);
    for (int i = 0; i < HOTBAR_SIZE; i++) {
        const InventorySlot& slot = _slots[HOTBAR_START + i];
        if (slot.itemId == -1) return true;
        if (slot.itemId == itemId && slot.count < maxStack) return true;
    }
    for (int i = 0; i < MAIN_STORAGE_SIZE; i++) {
        const InventorySlot& slot = _slots[MAIN_STORAGE_START + i];
        if (slot.itemId == -1) return true;
        if (slot.itemId == itemId && slot.count < maxStack) return true;
    }
    return false;
}

Int32 Player::addItemToInventory(Int32 itemId, Int32 count, std::vector<int>& changedSlots) {
    Int32 maxStack = ItemProperties::getMaxStackSize(itemId);
    // Merge into an existing stack -- hotbar before main storage -- then
    // fall back to the first empty slot, hotbar before main storage again.
    for (int i = 0; i < HOTBAR_SIZE && count > 0; i++) {
        InventorySlot& slot = _slots[HOTBAR_START + i];
        if (slot.itemId != itemId) continue;
        Int32 room = maxStack - slot.count;
        if (room <= 0) continue;
        Int32 added = std::min(room, count);
        slot.count += added;
        count -= added;
        changedSlots.push_back(HOTBAR_START + i);
    }
    for (int i = 0; i < MAIN_STORAGE_SIZE && count > 0; i++) {
        InventorySlot& slot = _slots[MAIN_STORAGE_START + i];
        if (slot.itemId != itemId) continue;
        Int32 room = maxStack - slot.count;
        if (room <= 0) continue;
        Int32 added = std::min(room, count);
        slot.count += added;
        count -= added;
        changedSlots.push_back(MAIN_STORAGE_START + i);
    }
    for (int i = 0; i < HOTBAR_SIZE && count > 0; i++) {
        InventorySlot& slot = _slots[HOTBAR_START + i];
        if (slot.itemId != -1) continue;
        Int32 added = std::min(maxStack, count);
        slot = {itemId, added};
        count -= added;
        changedSlots.push_back(HOTBAR_START + i);
    }
    for (int i = 0; i < MAIN_STORAGE_SIZE && count > 0; i++) {
        InventorySlot& slot = _slots[MAIN_STORAGE_START + i];
        if (slot.itemId != -1) continue;
        Int32 added = std::min(maxStack, count);
        slot = {itemId, added};
        count -= added;
        changedSlots.push_back(MAIN_STORAGE_START + i);
    }
    return count;
}

const InventorySlot& Player::getCarriedItem() const {
    return _carriedItem;
}

void Player::setCarriedItem(Int32 itemId, Int32 count) {
    _carriedItem = {itemId, count};
}

int Player::getContainerStateId() const {
    return _containerStateId;
}

int Player::advanceContainerStateId() {
    return ++_containerStateId;
}

bool Player::isDragActive() const {
    return _dragActive;
}

bool Player::isDragRightClick() const {
    return _dragRightClick;
}

void Player::beginDrag(bool rightClick) {
    _dragActive = true;
    _dragRightClick = rightClick;
    _dragSlots.clear();
}

void Player::addDragSlot(int slot) {
    for (int existing : _dragSlots) {
        if (existing == slot) return;
    }
    _dragSlots.push_back(slot);
}

const std::vector<int>& Player::getDragSlots() const {
    return _dragSlots;
}

void Player::clearDrag() {
    _dragActive = false;
    _dragSlots.clear();
}