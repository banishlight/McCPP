#pragma once
#include <Standards.hpp>
#include <vector>

void initCrypto();
void cleanupCrypto();
std::vector<Byte> generatePublicKey();
std::vector<Byte> decryptWithPrivateKey(const std::vector<Byte>& data);
std::vector<Byte> generateVerifyToken();

#ifdef LINUX

#endif

#ifdef WINDOWS

#endif