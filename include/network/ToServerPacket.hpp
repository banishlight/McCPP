#pragma once
#include <Standards.hpp>
#include <network/Connection.hpp>

// Handshaking State
void server_Handshake(Connection conn, void* data);

// Status State
void server_Status_Request(Connection conn);
void server_Ping_Request(void* packetData);

// Login State
void server_Login_Start(void* packetData);
void server_Encryption_Response(void* packetData);
void server_Login_Plugin_Response(void* packetData);
void server_Login_Acknowledged(void);
void server_Cookie_Reponse_login(void* packetData);

// Configuration State
void server_Client_Information(void* packetData);
void server_Cookie_Response(void* packetData);
void server_Plugin_Message(void* packetData);
void server_Acknowledge_Finish_Config(void);
void server_Keep_Alive(void* packetData);
void server_Pong(void* packetData);
void server_Resource_Pack_Response(void* packetData);
void server_Known_Packs(void* packetData);

// Typedef function pointer array for play state here