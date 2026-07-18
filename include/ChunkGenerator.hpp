#pragma once
#include <Standards.hpp>
#include <Chunk.hpp>
#include <memory>

// Base class for something that can produce a Chunk for a given column.
// Subclass and hand an instance to World to add a new world type (flat,
// real terrain generation, etc.) without World needing to know the details.
class ChunkGenerator {
    public:
        virtual ~ChunkGenerator() = default;
        virtual std::shared_ptr<Chunk> generate(int chunkX, int chunkZ) = 0;
        virtual string getName() const = 0;
};