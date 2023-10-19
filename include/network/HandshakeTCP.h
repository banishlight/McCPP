#pragma once
#include <string>
class HandshakeTCP
{
private:
	//MinecraftServer
	//NetworkManager

public:


	void OnDisconnect(std::string reason);
};
void ProcessHandshake(UInt32 mcVersion, UInt16 port, const char* ip);