#pragma once
#include <Standards.hpp>
#include <vector>

std::vector<Byte> decompressData(const std::vector<Byte>& compressedData);
std::vector<Byte> compressData(const std::vector<Byte>& data);