#pragma once
#include <Standards.hpp>

// Minimal placeholder world: a single hardcoded void dimension, no chunk data
// yet. Exists as the query boundary a real multi-chunk world will sit behind
// later, so callers already ask "the world" for this instead of hardcoding
// spawn constants inline.
class World {
    public:
        static World& getInstance();
        string getDimensionName() const;
        double getSpawnX() const;
        double getSpawnY() const;
        double getSpawnZ() const;
        float getSpawnYaw() const;
        Int64 getHashedSeed() const;
        bool isFlat() const;
    private:
        World() = default;
        string _dimensionName = "minecraft:overworld";
        double _spawnX = 0.5;
        double _spawnY = 100.0;
        double _spawnZ = 0.5;
        float _spawnYaw = 0.0f;
        Int64 _hashedSeed = 0;
        bool _isFlat = false;
};