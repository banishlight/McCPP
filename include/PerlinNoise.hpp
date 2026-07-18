#pragma once
#include <Standards.hpp>
#include <array>

// Classic Ken Perlin "Improved Noise" (gradient noise): a public-domain,
// patent-expired technique, not vanilla Minecraft's implementation or tuned
// data. Deterministically seeded so world generation is reproducible per seed.
class PerlinNoise {
    public:
        explicit PerlinNoise(Int64 seed);
        // Single octave, range approximately [-1, 1].
        double noise(double x, double y, double z) const;
        // Layered fractal Brownian motion: `octaves` layers of decreasing
        // amplitude (persistence) and increasing frequency (lacunarity).
        double fbm(double x, double y, double z, int octaves, double persistence, double lacunarity) const;
    private:
        static double fade(double t);
        static double lerp(double t, double a, double b);
        static double grad(int hash, double x, double y, double z);
        std::array<int, 512> _permutation;
};