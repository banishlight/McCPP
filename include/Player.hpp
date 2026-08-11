#pragma once
#include <Standards.hpp>
#include <vector>
#include <array>
#include <set>
#include <utility>

// itemId -1 = empty slot.
struct InventorySlot {
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
        // Real vanilla player-inventory (Window ID 0) slot layout: 0 =
        // crafting result, 1-4 = crafting grid, 5-8 = armor, 9-35 = main
        // storage, 36-44 = hotbar, 45 = offhand. One flat array is the single
        // source of truth for all of it -- the hotbar is just a named
        // sub-range, not a separate copy.
        static constexpr int TOTAL_SLOTS = 46;
        static constexpr int HOTBAR_START = 36;
        static constexpr int MAIN_STORAGE_START = 9;
        static constexpr int MAIN_STORAGE_SIZE = 27;
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
        // From Player_Command_p (start/stop sneaking, start/stop sprinting) --
        // relayed to other players via entity metadata (Entity Flags bitmask +
        // Pose) so their client renders the crouch/sprint animation.
        bool isSneaking() const;
        void setSneaking(bool sneaking);
        bool isSprinting() const;
        void setSprinting(bool sprinting);
        // Set from the player's own feet-block on every position update
        // (Set_Player_Position_p et al.) -- drives the swimming Pose sent to
        // other players. This project has no server-side movement physics at
        // all, so the actual swim/buoyancy slowdown is the real client's own
        // local behavior once it sees a real water block; this flag only
        // controls what OTHER clients render for this player.
        bool isInWater() const;
        void setInWater(bool inWater);
        // Wire value for the Pose metadata field (Set_Player_Pose_Metadata_p):
        // in-water beats sneaking beats standing, matching vanilla's own
        // precedence. The single place pose-priority logic lives.
        int getPose() const;
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
        // Full 46-slot inventory (crafting/armor/main-storage/hotbar/offhand,
        // see TOTAL_SLOTS above). No Click Container handling yet, so
        // non-Creative slots can only ever be set server-side (e.g. the
        // starting stack seeded at join) or via the Creative direct-set path
        // -- a Survival client can't rearrange its own inventory server-side
        // yet.
        const std::array<InventorySlot, TOTAL_SLOTS>& getInventory() const;
        const InventorySlot& getSlot(int index) const;
        void setSlot(int index, Int32 itemId, Int32 count);
        // Thin views over the hotbar sub-range (indices HOTBAR_START..
        // HOTBAR_START+HOTBAR_SIZE-1 of the same array above) -- kept so
        // existing hotbar-only call sites (ground-item pickup, bucket use)
        // don't need to know about the wider inventory at all.
        std::array<InventorySlot, HOTBAR_SIZE> getHotbar() const;
        void setHotbarSlot(int index, Int32 itemId, Int32 count);
        int getSelectedSlot() const;
        void setSelectedSlot(int slot);
        // Read-only capacity check (existing partial stack with room, or an
        // empty slot) -- callers should check this before claiming a ground
        // item, so a claim never has to be rolled back for lack of space.
        // Hotbar then main storage (matches addItemToInventory's scope/order
        // below).
        bool hasRoomFor(Int32 itemId) const;
        // Merges into an existing partial stack of itemId first, then the
        // first empty slot -- each phase checks the hotbar before main
        // storage, mirroring real vanilla's own raw inventory indexing
        // (hotbar occupies the first 9 slots there, storage the rest, and
        // pickup always searches in that order). Returns leftover count that
        // didn't fit (0 on full success) and appends the absolute slot index
        // (not hotbar-relative) of every slot it changed to changedSlots.
        Int32 addItemToInventory(Int32 itemId, Int32 count, std::vector<int>& changedSlots);
        // The server's authoritative view of the mouse cursor's held stack
        // (Click_Container_p, Play.cpp) -- itemId -1 means nothing carried.
        const InventorySlot& getCarriedItem() const;
        void setCarriedItem(Int32 itemId, Int32 count);
        // A server-managed sequence number the client must echo back in
        // Click_Container_p to prove it's acting on the server's current view
        // of the container -- see docs/network-protocol.md's Click Container/
        // Set Container Slot sections. Every Set_Container_Content_p/
        // Set_Container_Slot_p sent to this player calls advanceContainerStateId()
        // for a fresh value; Click_Container_p compares the client's claimed
        // id against getContainerStateId() before trusting the click.
        int getContainerStateId() const;
        int advanceContainerStateId();
        // Mode 5 (drag/paint) click handling: real vanilla splits one drag
        // gesture across several packets -- a start (slot -999), one per
        // slot painted over, and an end (slot -999) -- so the server has to
        // remember which slots were painted and which mouse button started
        // the gesture across that whole sequence. beginDrag resets the
        // accumulated slot list; addDragSlot appends a slot (ignored if
        // already present, since real vanilla never re-paints the same
        // slot twice in one gesture, but nothing here should trust that
        // blindly); clearDrag ends the session (called whether it finished
        // normally or was reset by an out-of-order packet).
        bool isDragActive() const;
        bool isDragRightClick() const;
        void beginDrag(bool rightClick);
        void addDragSlot(int slot);
        const std::vector<int>& getDragSlots() const;
        void clearDrag();
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
        bool _sneaking = false;
        bool _sprinting = false;
        bool _inWater = false;
        std::set<std::pair<int, int>> _loadedChunks;
        int _centerChunkX = 0; // matches the fixed spawn chunk (0,0)
        int _centerChunkZ = 0;
        std::array<InventorySlot, TOTAL_SLOTS> _slots{};
        int _selectedSlot = 0;
        InventorySlot _carriedItem{};
        int _containerStateId = 0;
        bool _dragActive = false;
        bool _dragRightClick = false;
        std::vector<int> _dragSlots;
};