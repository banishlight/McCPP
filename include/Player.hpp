#pragma once
#include <Standards.hpp>
#include <vector>
#include <atomic>

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
};