#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#include <network/DataBuffer.hpp>
#include <Standards.hpp>
#include <network/IPAddress.hpp>



class Socket
{
public:
	enum Status { CONNECT, DISCONNECT, ERROR };
	enum Type { TCP, UDP };
	//Methods
	Socket(Socket&& rhs) = default; // std move constructor
	Socket& operator=(Socket&& rhs) = default; //set move operator
	Socket(Type type);
	void SetStatus(Status status);
	void Disconnect();
	size Send(const string& data);
	size Send(DataBuffer& buffer); //todo implement DataBuffer
	bool Connect(const string& ip, UInt16 port);

	//Virtuals
	virtual ~Socket();
	virtual bool Connect(const IPAddress& addr, UInt16 port) = 0;
	virtual size Send(const Byte* data, size size) = 0;
	virtual DataBuffer Receive(size amount) = 0;
	virtual size Receive(DataBuffer& buffer, size amount) = 0;
	virtual void SetBlocking(bool block)=0;
	//Attributes
	bool isBlocking;
	Status status;
	Type type;
	Int32 sockHandle;
	//typedef smart pointer
	typedef std::shared_ptr<Socket> SocketPtr;

};
