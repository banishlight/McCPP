#pragma once
#include <Standards.hpp>

// Will need to support zlib compression in the future
// Thread safety!

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
		std::unique_ptr<Incoming_Packet> receivePacket();
		bool packetAvailable();
		void sendPacket(const Outgoing_Packet* packet);
		void setBlocking(bool blocking);
		bool isBlocking() const;

    protected:
	// Independent variables per OS
	#ifdef LINUX
		int fd = -1;
		bool blocking = true;
	#endif
	#ifdef WINDOWS

	#endif
};
