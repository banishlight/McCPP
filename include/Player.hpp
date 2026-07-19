#pragma once
#include <Standards.hpp>
#include <vector>
#include <array>
#include <set>
#include <utility>

// itemId -1 = empty slot.
struct HotbarSlot {
    Int32 itemId = -1;
    Int32 count = 0;
};

// Player/gameplay-related state for a connected client. Deliberately separate from
// Connection, which stays scoped to socket/protocol concerns (state, compression,
// encryption). Populated incrementally as login/configuration packets arrive.
class Player {
    public:
        static constexpr int HOTBAR_SIZE = 9;
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
        // Hotbar only -- no full inventory/armor/offhand tracking, and no
        // Click Container handling, so slots can only ever be set server-side
        // (e.g. the starting stack seeded at join), never rearranged by the client.
        const std::array<HotbarSlot, HOTBAR_SIZE>& getHotbar() const;
        void setHotbarSlot(int index, Int32 itemId, Int32 count);
        int getSelectedSlot() const;
        void setSelectedSlot(int slot);
        // Read-only capacity check (existing partial stack with room, or an
        // empty slot) -- callers should check this before claiming a ground
        // item, so a claim never has to be rolled back for lack of space.
        bool hasRoomFor(Int32 itemId) const;
        // Merges into an existing partial stack of itemId first, then the
        // first empty slot. Returns leftover count that didn't fit (0 on full
        // success) and appends the index of every slot it changed to changedSlots.
        Int32 addItemToHotbar(Int32 itemId, Int32 count, std::vector<int>& changedSlots);
    private:
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
        std::array<HotbarSlot, HOTBAR_SIZE> _hotbar{};
        int _selectedSlot = 0;
};