#pragma once
#include <network/DataBuffer.hpp>
#include <network/Socket.hpp>
#include <Standards.hpp>
class TCPSocket : public Socket {
	
private:
	IPAddress remoteIp;
	UInt16 port;
	sockaddr_in remoteAddr;
public:
	TCPSocket();
	bool Connect(const IPAddress& addr, UInt16 port) override;
	size Send(const Byte* data, size size) override;
	size Receive(DataBuffer& buffer, size amount) override;
	DataBuffer Receive(size amount) override;
	void Disconnect();
};
