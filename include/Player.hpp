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

// One entry of a Mojang game profile's "properties" array (currently always
// at most one, named "textures", carrying the base64-encoded skin/cape JSON
// blob plus Mojang's signature over it) -- shared by Login_Success_p (tells
// this player's own client its skin) and the future Player Info Update
// broadcast (tells other players' clients the same thing).
struct PlayerProfileProperty {
    std::string name;
    std::string value;
    std::string signature;
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
        // Populated from Mojang's hasJoined response once online-mode
        // verification completes (Encryption_Response_p::deserialize) --
        // empty until then, which clients render as a default Steve/Alex skin.
        const std::vector<PlayerProfileProperty>& getProfileProperties() const;
        void setProfileProperties(std::vector<PlayerProfileProperty> properties);
        // "Displayed Skin Parts" bitmask from Client Information (cape/jacket/
        // sleeves/pants/hat) -- relayed to other players via player-entity
        // metadata so their client renders the right skin layers for this
        // player. Defaults to all-enabled (0x7F) since Client Information
        // always arrives before Play state in practice.
        Byte getSkinParts() const;
        void setSkinParts(Byte skinParts);
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
        std::vector<PlayerProfileProperty> _profileProperties;
        Byte _skinParts = 0x7F;
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