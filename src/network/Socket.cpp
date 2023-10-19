#include "../../include/Standards.h"
#include "../../include/network/Socket.h"
#include "../../include/network/IPAddress.h"
#include <fcntl.h>

Socket::Socket(Type type) : sockHandle(-1), type(type),isBlocking(false),status(DISCONNECT)
{
	
}

Socket::~Socket()
{
	Disconnect();
}
void Socket::SetBlocking(bool block)
{
	UInt64 mode = block ? 0 : 1;
	Int32 opts = fcntl(sockHandle, F_GETFL);
	if (opts < 0)return;
	if(block)
	{
		opts |= O_NONBLOCK;
	}
	else
	{
		opts &= ~O_NONBLOCK;
	}
	fcntl(sockHandle, F_SETFL, opts);
	isBlocking = block;
}

bool Socket::Connect(const IPAddress& addr, UInt16 port)
{
	IPAddress address(addr);
	return Connect(address, port);
}

size Socket::Send(DataBuffer& buffer)
{
	string data = buffer.ToString();
	return this->Send(reinterpret_cast<const Byte*>(data.c_str()), data.length());
}

void Socket::Disconnect()
{
	if(sockHandle != -1)
	{
		close(sockHandle);
	}
	status = DISCONNECT;
}





