#include <network/Crypto.hpp>
#include <Standards.hpp>
#include <Console.hpp>
#include <vector>
#include <sstream>
#include <iomanip>


#ifdef LINUX
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/rand.h>

static EVP_PKEY* server_keypair = nullptr;

void initCrypto() {
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    Console::getConsole().Entry("Cryptographic subsystem initialized");
}

void cleanupCrypto() {
    if (server_keypair) {
        EVP_PKEY_free(server_keypair);
        server_keypair = nullptr;
    }
    EVP_cleanup();
    ERR_free_strings();
    Console::getConsole().Entry("Cryptographic subsystem cleaned up");
}

std::vector<Byte> generatePublicKey() {
    if (!server_keypair) {
        // Initialize OpenSSL
        ERR_load_crypto_strings();
        OpenSSL_add_all_algorithms();

        // Generate RSA key pair -- only once per process lifetime (see header comment).
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
        if (!ctx) {
            Console::getConsole().Error("Failed to create RSA context");
            return std::vector<Byte>();
        }

        if (EVP_PKEY_keygen_init(ctx) <= 0) {
            Console::getConsole().Error("Failed to initialize RSA key generation");
            EVP_PKEY_CTX_free(ctx);
            return std::vector<Byte>();
        }

        if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 1024) <= 0) {
            Console::getConsole().Error("Failed to set RSA key size");
            EVP_PKEY_CTX_free(ctx);
            return std::vector<Byte>();
        }

        if (EVP_PKEY_keygen(ctx, &server_keypair) <= 0) {
            Console::getConsole().Error("Failed to generate RSA key pair");
            EVP_PKEY_CTX_free(ctx);
            return std::vector<Byte>();
        }

        EVP_PKEY_CTX_free(ctx);
    }

    // Extract public key in X.509 DER format (what Minecraft expects)
    unsigned char* pub_key_der = nullptr;
    int pub_key_len = i2d_PUBKEY(server_keypair, &pub_key_der);
    
    if (pub_key_len <= 0) {
        Console::getConsole().Error("Failed to encode public key");
        return std::vector<Byte>();
    }
    
    // Convert to vector
    std::vector<Byte> public_key_bytes(pub_key_der, pub_key_der + pub_key_len);
    
    // Free the allocated memory
    OPENSSL_free(pub_key_der);
    
    Console::getConsole().Entry("Generated " + std::to_string(pub_key_len) + " byte RSA public key");
    
    return public_key_bytes;
}

std::vector<Byte> decryptWithPrivateKey(const std::vector<Byte>& data) {
    if (!server_keypair) {
        Console::getConsole().Error("No server key pair available for decryption");
        return std::vector<Byte>();
    }
    
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(server_keypair, nullptr);
    if (!ctx) {
        Console::getConsole().Error("Failed to create decryption context");
        return std::vector<Byte>();
    }
    
    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        Console::getConsole().Error("Failed to initialize decryption");
        EVP_PKEY_CTX_free(ctx);
        return std::vector<Byte>();
    }
    
    size_t outlen;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, data.data(), data.size()) <= 0) {
        Console::getConsole().Error("Failed to determine decryption output length");
        EVP_PKEY_CTX_free(ctx);
        return std::vector<Byte>();
    }
    
    std::vector<Byte> decrypted(outlen);
    if (EVP_PKEY_decrypt(ctx, decrypted.data(), &outlen, data.data(), data.size()) <= 0) {
        Console::getConsole().Error("Failed to decrypt data");
        EVP_PKEY_CTX_free(ctx);
        return std::vector<Byte>();
    }
    
    EVP_PKEY_CTX_free(ctx);
    decrypted.resize(outlen);
    return decrypted;
}

std::vector<Byte> generateVerifyToken() {
    std::vector<Byte> token(4);
    if (RAND_bytes(token.data(), 4) != 1) {
        Console::getConsole().Error("Failed to generate random verify token");
        return std::vector<Byte>();
    }
    return token;
}

bool verifyToken(const std::vector<Byte>& expected, const std::vector<Byte>& actual) {
    return !expected.empty() && expected == actual;
}

std::string generateServerId() {
    // Encryption Request's Server ID field is a String(20) -- the vanilla
    // client rejects (decode-error-disconnects) anything longer, so this must
    // stay at or under 20 characters. 10 random bytes -> 20 hex characters.
    Byte raw[10];
    if (RAND_bytes(raw, 10) != 1) {
        Console::getConsole().Error("Failed to generate random server ID");
        return "";
    }
    static const char hexDigits[] = "0123456789abcdef";
    std::string id(20, '0');
    for (int i = 0; i < 10; i++) {
        id[i * 2] = hexDigits[(raw[i] >> 4) & 0x0F];
        id[i * 2 + 1] = hexDigits[raw[i] & 0x0F];
    }
    return id;
}

std::string computeServerHash(const std::string& serverId, const std::vector<Byte>& sharedSecret, const std::vector<Byte>& publicKeyDer) {
    std::vector<Byte> input;
    input.insert(input.end(), serverId.begin(), serverId.end());
    input.insert(input.end(), sharedSecret.begin(), sharedSecret.end());
    input.insert(input.end(), publicKeyDer.begin(), publicKeyDer.end());

    std::vector<Byte> digest(EVP_MAX_MD_SIZE);
    unsigned int digestLen = 0;
    EVP_MD_CTX* mdCtx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdCtx, EVP_sha1(), nullptr);
    EVP_DigestUpdate(mdCtx, input.data(), input.size());
    EVP_DigestFinal_ex(mdCtx, digest.data(), &digestLen);
    EVP_MD_CTX_free(mdCtx);
    digest.resize(digestLen);

    // Java's BigInteger(digest).toString(16): two's-complement negate if the
    // sign bit is set, strip leading all-zero bytes, and never zero-pad the
    // most significant remaining hex digit (only later bytes get 2-digit padding).
    bool negative = (digest[0] & 0x80) != 0;
    if (negative) {
        int carry = 1;
        for (int i = static_cast<int>(digest.size()) - 1; i >= 0; i--) {
            int val = (~digest[i] & 0xFF) + carry;
            digest[i] = static_cast<Byte>(val & 0xFF);
            carry = (val > 0xFF) ? 1 : 0;
        }
    }

    size_t start = 0;
    while (start < digest.size() - 1 && digest[start] == 0) start++;

    std::ostringstream oss;
    oss << std::hex << static_cast<int>(digest[start]);
    for (size_t i = start + 1; i < digest.size(); i++) {
        oss << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return negative ? ("-" + oss.str()) : oss.str();
}

std::vector<long> generateRandomUUID() {
    Byte raw[16];
    if (RAND_bytes(raw, 16) != 1) {
        Console::getConsole().Error("Failed to generate random UUID");
        return {0, 0};
    }
    raw[6] = (raw[6] & 0x0F) | 0x40; // version 4
    raw[8] = (raw[8] & 0x3F) | 0x80; // variant RFC 4122

    long mostSignificant = 0, leastSignificant = 0;
    for (int i = 0; i < 8; i++) mostSignificant = (mostSignificant << 8) | static_cast<long>(raw[i]);
    for (int i = 8; i < 16; i++) leastSignificant = (leastSignificant << 8) | static_cast<long>(raw[i]);
    return {mostSignificant, leastSignificant};
}

StreamCipher::StreamCipher(const std::vector<Byte>& sharedSecret, bool encrypting) {
    _encrypting = encrypting;
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        Console::getConsole().Error("StreamCipher: Failed to create cipher context");
        return;
    }
    // Minecraft's protocol uses the shared secret as both the AES key and the IV
    int rc = encrypting
        ? EVP_EncryptInit_ex(ctx, EVP_aes_128_cfb8(), nullptr, sharedSecret.data(), sharedSecret.data())
        : EVP_DecryptInit_ex(ctx, EVP_aes_128_cfb8(), nullptr, sharedSecret.data(), sharedSecret.data());
    if (rc != 1) {
        Console::getConsole().Error("StreamCipher: Failed to initialize cipher");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    _ctx = ctx;
}

StreamCipher::~StreamCipher() {
    if (_ctx) {
        EVP_CIPHER_CTX_free(static_cast<EVP_CIPHER_CTX*>(_ctx));
    }
}

std::vector<Byte> StreamCipher::process(const std::vector<Byte>& data) {
    if (!_ctx || data.empty()) {
        return data;
    }
    EVP_CIPHER_CTX* ctx = static_cast<EVP_CIPHER_CTX*>(_ctx);
    std::vector<Byte> out(data.size());
    int outlen = 0;
    int rc = _encrypting
        ? EVP_EncryptUpdate(ctx, out.data(), &outlen, data.data(), static_cast<int>(data.size()))
        : EVP_DecryptUpdate(ctx, out.data(), &outlen, data.data(), static_cast<int>(data.size()));
    if (rc != 1) {
        Console::getConsole().Error("StreamCipher::process(): Cipher update failed");
        return std::vector<Byte>();
    }
    out.resize(outlen);
    return out;
}
#endif

#ifdef WINDOWS
void initCrypto() {

}

void cleanupCrypto() {

}

std::vector<Byte> generatePublicKey() {

}

std::vector<Byte> decryptWithPrivateKey(const std::vector<Byte>& data) {

}
#endif