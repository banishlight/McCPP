#pragma once
#include <string>

class LoginServer
{
public:
	LoginServer();
	void Update();
	void CloseConnection(std::string reason);
	void TryAcceptPlayer();
	void OnDisconnect();
	std::string GetConnectionInfo();
	void ProcessLoginStart();
	void ProcessEncryptionResponse();
	//NetworkManager

protected:
	//GameProfile;

private:
	//AtomicInteger
	//Logger
	//Random
	//byte[]
	//MinecraftServer
	//NetHandlerLoginServer.LoginState
	//GameProfile
	//SecretKey
	//EntityPlayerMP
	//int connectionTimer;
	//std::string serverId;
};
