#include <network/Crypto.hpp>
#include <Standards.hpp>
#include <Console.hpp>
#include <vector>


#ifdef LINUX
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/rand.h>

static EVP_PKEY* server_keypair = nullptr;
static std::vector<Byte> current_verify_token;

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
    current_verify_token.clear();
    EVP_cleanup();
    ERR_free_strings();
    Console::getConsole().Entry("Cryptographic subsystem cleaned up");
}

std::vector<Byte> generatePublicKey() {
    // Initialize OpenSSL
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    
    // Generate RSA key pair
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
    // Generate 4 random bytes for verify token
    current_verify_token.resize(4);
    if (RAND_bytes(current_verify_token.data(), 4) != 1) {
        Console::getConsole().Error("Failed to generate random verify token");
        return std::vector<Byte>();
    }
    return current_verify_token;
}

bool verifyToken(const std::vector<Byte>& token) {
    return !current_verify_token.empty() && token == current_verify_token;
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