#pragma once
#include <Standards.hpp>
#include <vector>

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
    private:
        string _username;
        std::vector<long> _uuid;
        int _viewDistance = 10;
};