#pragma once
#include <Standards.hpp>
#include <vector>
#include <atomic>
#include <set>
#include <utility>

// Player/gameplay-related state for a connected client. Deliberately separate from
// Connection, which stays scoped to socket/protocol concerns (state, compression,
// encryption). Populated incrementally as login/configuration packets arrive.
class Player {
    public:
        Player();
        string getUsername() const;
        void setUsername(const string& username);
        std::vector<long> getUUID() const;
        void setUUID(std::vector<long> uuid);
        int getViewDistance() const;
        void setViewDistance(int viewDistance);
        int getEntityId() const;
        int getGamemode() const;
        void setGamemode(int gamemode);
        double getX() const;
        double getY() const;
        double getZ() const;
        float getYaw() const;
        float getPitch() const;
        void setPosition(double x, double y, double z);
        void setRotation(float yaw, float pitch);
        // Chunk columns currently sent to this player's client, and the chunk
        // they were last centered on -- lets movement handling diff against
        // what's actually loaded instead of resending everything every move.
        bool hasChunkLoaded(int chunkX, int chunkZ) const;
        void markChunkLoaded(int chunkX, int chunkZ);
        void markChunkUnloaded(int chunkX, int chunkZ);
        const std::set<std::pair<int, int>>& getLoadedChunks() const;
        int getCenterChunkX() const;
        int getCenterChunkZ() const;
        void setCenterChunk(int chunkX, int chunkZ);
    private:
        static std::atomic<int> _nextEntityId;
        string _username;
        std::vector<long> _uuid;
        int _viewDistance = 10;
        int _entityId;
        int _gamemode = 0; // 0: Survival, 1: Creative, 2: Adventure, 3: Spectator
        double _x = 0.0;
        double _y = 0.0;
        double _z = 0.0;
        float _yaw = 0.0f;
        float _pitch = 0.0f;
        std::set<std::pair<int, int>> _loadedChunks;
        int _centerChunkX = 0; // matches the fixed spawn chunk (0,0)
        int _centerChunkZ = 0;
};