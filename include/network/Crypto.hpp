#pragma once
#include <Standards.hpp>
#include <vector>

void initCrypto();
void cleanupCrypto();
// Generates the server's RSA keypair on first call and memoizes it for the
// rest of the process's lifetime (one keypair per server run, matching
// vanilla) -- later calls just re-export the same DER bytes. This matters
// beyond performance: the Mojang join-hash (computeServerHash) must be
// computed over the exact same public key DER that was sent to the client in
// Encryption_Request_p, so regenerating here would silently break every login.
std::vector<Byte> generatePublicKey();
std::vector<Byte> decryptWithPrivateKey(const std::vector<Byte>& data);
// Random 4-byte verify token for one connection attempt. Pure -- callers own
// storing it (see Connection::setVerifyToken) and pass it back to verifyToken().
std::vector<Byte> generateVerifyToken();

// Random RFC 4122 v4 UUID (2 longs, big-endian, matching deserializeUUID's
// representation) -- for entities with no client-supplied UUID (unlike
// players, who arrive with one from Login Start).
std::vector<long> generateRandomUUID();
// Opaque random per-connection identifier, sent in Encryption_Request_p and
// reused later (Connection::getServerId()) as the "serverId" input to the
// Mojang join-hash (see computeServerHash) -- content is arbitrary, it just
// has to match between the two uses for one login attempt.
std::string generateServerId();

// Mojang's join-hash: SHA1(serverId + sharedSecret + publicKeyDer), formatted
// as a signed hex string matching Java's `new BigInteger(digest).toString(16)`
// (two's-complement negate if the top bit is set, no leading zero padding on
// the most significant hex digit) -- required by sessionserver.mojang.com's
// hasJoined endpoint. Verified against the standard published test vectors
// (SHA1("Notch")/"jeb_"/"simon"), not guessed from memory.
std::string computeServerHash(const std::string& serverId, const std::vector<Byte>& sharedSecret, const std::vector<Byte>& publicKeyDer);
// Pure comparison of a decrypted verify token against the one this connection
// generated (Connection::getVerifyToken()) -- no hidden global state, so
// concurrent logins from different connections can never clobber each other.
bool verifyToken(const std::vector<Byte>& expected, const std::vector<Byte>& actual);

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