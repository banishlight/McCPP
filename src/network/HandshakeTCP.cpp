#include "../../include/Standards.h"
#include "../../include/network/HandshakeTCP.h"
#include "../../include/network/VarIntLong.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <iostream>



void ProcessHandshake(UInt32 mcVersion,UInt16 port, const char* ip)
{

	//pack all data to send to server

	
	UInt8 statusReqPacket[2] = { 1,0 };

	//socket stuff
	int sock = 0;
	int readVal, clientfd;
	struct sockaddr_in serverAddress;

	if((sock = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		std::cout << "Socket error try restarting server" << std::endl;
		return;
	}
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);




	// Convert IPv4 and IPv6 addresses from text to binary
	if (inet_pton(AF_INET, ip, &serverAddress.sin_addr)
		<= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
		return;
	}

	if ((clientfd
		= connect(sock, (struct sockaddr*)&serverAddress,
			sizeof(serverAddress)))
		< 0) {
		printf("\nConnection Failed \n");
		return;
	}


	close(clientfd);
	
}



