#pragma once
#include <Standards.hpp>

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
	
        bool isValid() const;
		std::vector<Byte> receivePacket();
		bool packetAvailable();
		void sendPacket(std::vector<Byte> data);
		void setBlocking(bool blocking);
		bool isBlocking() const;
		void close();
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
};
