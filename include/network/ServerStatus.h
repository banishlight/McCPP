#pragma once
#include "../../include/Standards.h"
class ServerStatus
{
public:
	ServerStatus();
	//NetHandlerStatusServer
	void OnDisconnect(string reason);
	void ProcessServerQuery();
	void ProcessPing();
private:
	// TextComponent EXIT_MESSAGE;
	// MinecraftServer server;
	//NetworkManager networkMan;
	//static bool handled;
};