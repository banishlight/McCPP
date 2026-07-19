#pragma once
#include <Standards.hpp>
#include <vector>

void initCrypto();
void cleanupCrypto();
std::vector<Byte> generatePublicKey();
std::vector<Byte> decryptWithPrivateKey(const std::vector<Byte>& data);
std::vector<Byte> generateVerifyToken();

// Random RFC 4122 v4 UUID (2 longs, big-endian, matching deserializeUUID's
// representation) -- for entities with no client-supplied UUID (unlike
// players, who arrive with one from Login Start).
std::vector<long> generateRandomUUID();
// Compares a decrypted verify token against the one generated for this connection attempt.
bool verifyToken(const std::vector<Byte>& token);

// AES-128/CFB-8 stream cipher. Per the Minecraft protocol, the shared secret
// established during login is used as both the AES key and the IV, and one
// instance is needed per direction (encrypt vs decrypt) since each is a
// stateful, ordered byte stream.
class StreamCipher {
    public:
        StreamCipher(const std::vector<Byte>& sharedSecret, bool encrypting);
        ~StreamCipher();
        StreamCipher(const StreamCipher&) = delete;
        StreamCipher& operator=(const StreamCipher&) = delete;
        std::vector<Byte> process(const std::vector<Byte>& data);
    private:
        void* _ctx = nullptr;
        bool _encrypting = false;
};

#ifdef LINUX

#endif

#ifdef WINDOWS

#endif