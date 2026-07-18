#include <PerlinNoise.hpp>
#include <cmath>
#include <random>
#include <algorithm>

PerlinNoise::PerlinNoise(Int64 seed) {
    std::array<int, 256> base;
    for (int i = 0; i < 256; i++) base[i] = i;
    std::mt19937_64 rng(static_cast<uint64_t>(seed));
    std::shuffle(base.begin(), base.end(), rng);
    for (int i = 0; i < 512; i++) _permutation[i] = base[i & 255];
}

double PerlinNoise::fade(double t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

double PerlinNoise::lerp(double t, double a, double b) {
    return a + t * (b - a);
}

double PerlinNoise::grad(int hash, double x, double y, double z) {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double PerlinNoise::noise(double x, double y, double z) const {
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;
    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);
    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

    int A = _permutation[X] + Y, AA = _permutation[A] + Z, AB = _permutation[A + 1] + Z;
    int B = _permutation[X + 1] + Y, BA = _permutation[B] + Z, BB = _permutation[B + 1] + Z;

    return lerp(w,
        lerp(v,
            lerp(u, grad(_permutation[AA], x, y, z), grad(_permutation[BA], x - 1, y, z)),
            lerp(u, grad(_permutation[AB], x, y - 1, z), grad(_permutation[BB], x - 1, y - 1, z))),
        lerp(v,
            lerp(u, grad(_permutation[AA + 1], x, y, z - 1), grad(_permutation[BA + 1], x - 1, y, z - 1)),
            lerp(u, grad(_permutation[AB + 1], x, y - 1, z - 1), grad(_permutation[BB + 1], x - 1, y - 1, z - 1))));
}

double PerlinNoise::fbm(double x, double y, double z, int octaves, double persistence, double lacunarity) const {
    double total = 0.0;
    double frequency = 1.0;
    double amplitude = 1.0;
    double maxValue = 0.0;
    for (int i = 0; i < octaves; i++) {
        total += noise(x * frequency, y * frequency, z * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    return total / maxValue;
}