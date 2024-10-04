#include <Standards.hpp>
#include <network/TCPSocket.hpp>
#include <network/IPAddress.hpp>

TCPSocket::TCPSocket() :Socket(Socket::TCP), port((0)) {
	sockHandle = -1;
}

bool TCPSocket::Connect(const IPAddress& addr, const UInt16 port) {
	if (this->status == CONNECT) {
		return true;
	}
	struct addrinfo hints = { 0 }, * result = nullptr;
	if((sockHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <0) {
		perror("Could create socket!\n");
		return false;
	}
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo(addr.ToString().c_str(), std::to_string(port).c_str(), &hints, &result) != 0) {
		perror("Could not get address!\n");
		return false;
	}

	struct addrinfo* ptr = nullptr;
	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
		auto* sockaddr = (struct sockaddr_in*)ptr->ai_addr;
		if(::connect(sockHandle,(struct  sockaddr*)sockaddr,sizeof(struct sockaddr_in))!=0) {
			continue;
		}
		else {
			perror("Error in connecting to socket!!\n");
			return false;
		}
		remoteAddr = *sockaddr;
		break;
	}
	freeaddrinfo(result);
	if(!ptr) {
		perror("Connection error unknown\n");
		return false;
	}
	this->status = CONNECT;
	remoteIp = addr;
	return true;
}

size TCPSocket::Send(const Byte* data, size sz) {
	if(this->status != CONNECT) {
		return 0;
	}
	size sent = 0;
	while (sent < sz) {
		Int64 cur = ::send(sockHandle, reinterpret_cast<const char*>(data + sent), sz - sent, 0);
		if(cur <=0) {
			Disconnect();
			return 0;
		}
		sent += cur;
	}
	return sent;
}

size TCPSocket::Receive(DataBuffer& dataBuffer, size amount) {
	dataBuffer.Resize(amount);
	dataBuffer.SetReadOffset(0);

	int recvAmount = recv(sockHandle, (char*)&dataBuffer[0], amount, MSG_DONTWAIT);
	if(recvAmount <=0) { // Possible memory leak here?
		Int32 err = errno;
		if (err == EWOULDBLOCK) {
			dataBuffer.Clear();
			return 0;
		}
		Disconnect();
		dataBuffer.Clear();
		return 0;
	}
	dataBuffer.Resize(recvAmount);
	return recvAmount;
}

DataBuffer TCPSocket::Receive(size amount) {
	std::unique_ptr<char[]> buf(new char[amount]);
	int received = ::recv(sockHandle, buf.get(), amount, MSG_DONTWAIT);

	if(received <=0) {
		const Int32 err = errno;
		if(err == EWOULDBLOCK) {
			return DataBuffer();
		}
		Disconnect();
		return DataBuffer();
	}
	return DataBuffer(string(buf.get(),received));
}

void TCPSocket::Listen(int sockfd, int backlog) {
	if (::listen(sockfd, backlog) < 0) {
        perror("Error in listen");
    }
}

void TCPSocket::Disconnect() {
    if (sockHandle != -1) {
        ::close(sockHandle);
        sockHandle = -1;
        this->status = DISCONNECT;
    }
}