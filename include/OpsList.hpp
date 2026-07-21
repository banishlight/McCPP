#pragma once
#include <Standards.hpp>
#include <mutex>
#include <unordered_map>

// Minimal ops.json-backed permission-level store, keyed by dashless-hex UUID
// (not username, so it survives a player renaming their Mojang account).
// Console commands never consult this -- ConsoleCommandSender is always max
// permission, matching vanilla.
class OpsList {
    public:
        static OpsList& getInstance();
        void initialize();
        // 0 for anyone not present in ops.json (matches Player's implicit default).
        int getOpLevel(const string& uuidHex) const;
        // level 0 removes the entry entirely, matching /deop. Persists to
        // ops.json immediately.
        void setOpLevel(const string& uuidHex, const string& name, int level);
    private:
        OpsList() = default;
        struct Entry {
            string name;
            int level;
        };
        void save() const;
        mutable std::mutex _mutex;
        std::unordered_map<string, Entry> _entries;
};
