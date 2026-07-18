#include <Player.hpp>
#include <Console.hpp>

std::atomic<int> Player::_nextEntityId{1};

Player::Player() : _entityId(_nextEntityId++) {
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