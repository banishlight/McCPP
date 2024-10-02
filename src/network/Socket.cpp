#include <Standards.hpp>
#include <network/Socket.hpp>
#include <network/IPAddress.hpp>
#include <fcntl.h>

Socket::Socket(Type type) : isBlocking(false),
	status(DISCONNECT),
	type(type),   
    sockHandle(-1) 
{
	
}

Socket::~Socket()
{
	Disconnect();
}
void Socket::SetBlocking(bool block)
{
	//UInt64 mode = block ? 0 : 1; // Unused
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





