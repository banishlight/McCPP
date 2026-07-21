#pragma once
#include <Standards.hpp>
#include <vector>

std::vector<Byte> decompressData(const std::vector<Byte>& compressedData);
std::vector<Byte> compressData(const std::vector<Byte>& data);

// Gzip-wrapped variants (different header/trailer than the plain zlib framing
// above) -- level.dat is always gzip; packet compression stays plain zlib, so
// these are separate functions rather than a mode flag on the existing ones.
std::vector<Byte> decompressGzip(const std::vector<Byte>& compressedData);
std::vector<Byte> compressGzip(const std::vector<Byte>& data);