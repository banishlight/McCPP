#pragma once
#include <Standards.hpp>

class Chunk;
class World;

// Computes real sky + block light via BFS flood-fill. There's no block
// placing/breaking yet, so lighting only ever needs to be computed once, at
// chunk generation time -- never incrementally re-updated. Since max light
// level (15) is less than chunk width (16), a chunk's lighting is exactly
// correct using only its own block data plus its 8 immediate neighbors' --
// no wider propagation is ever possible.
class LightEngine {
    public:
        // Computes and stores sky + block light directly on `target`, using a
        // 3x3-chunk (48x48 block) BFS input region centered on it. Only
        // `target`'s own 16x16 portion of the result is written back.
        static void computeLighting(Chunk& target, World& world);
    private:
        static bool isOpaque(Int32 blockStateId);
        static int getLightEmission(Int32 blockStateId);
};