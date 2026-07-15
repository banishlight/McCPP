#pragma once
#include <Standards.hpp>
#include <network/Crypto.hpp>
#include <memory>

// Will need to support zlib compression in the future
// Thread safety!

// Socket must be manually closed before deconstructing
class Socket {
    public:
		#ifdef LINUX
        Socket(int fd);
		#endif
		#ifdef WINDOWS
		Socket(); // ??
		#endif
        ~Socket();
        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;
        Socket(Socket&&) noexcept = default;
        Socket& operator=(Socket&&) noexcept = default;

        bool isValid() const;
		std::vector<Byte> receivePacket();
		bool packetAvailable();
		void sendPacket(std::vector<Byte> data);
		void setBlocking(bool blocking);
		bool isBlocking() const;
		void close();
		// Switches all further reads/writes on this socket to AES/CFB-8, keyed by the
		// shared secret negotiated during login. Must be called on both ends at the
		// same point in the packet stream (right after Encryption Response).
		void enableEncryption(const std::vector<Byte>& sharedSecret);
		bool isEncrypted() const;
    protected:
		// Independent variables per OS
		#ifdef LINUX
			int fetchVarInt();
			bool isValidFD(int fd) const;
			int _fd = -1;
			bool _blocking = true;
		#endif
		#ifdef WINDOWS

		#endif
		bool _encrypted = false;
		std::unique_ptr<StreamCipher> _encryptCipher;
		std::unique_ptr<StreamCipher> _decryptCipher;
};
