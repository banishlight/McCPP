#pragma once
#include "DataBuffer.h"
#include "Socket.h"
#include "../Standards.h"
class TCPSocket : public Socket
{
	
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
};
