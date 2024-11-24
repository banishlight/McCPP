#pragma once
#include <Standards.hpp>
#include <network/VarIntLong.hpp>

// Handshaking State
void server_Handshake(void* packet);

// Status State
void server_Status_Request(void);
void server_Ping_Request(void* packet);

// Login State
void server_Login_Start(void* packet);
void server_Encryption_Response(void* packet);
void server_Login_Plugin_Response(void* packet);
void server_Login_Acknowledged(void);
void server_Cookie_Reponse_login(void* packet);

// Configuration State
void server_Client_Information(void* packet);
void server_Cookie_Response(void* packet);
void server_Plugin_Message(void* packet);
void server_Acknowledge_Finish_Config(void);
void server_Keep_Alive(void* packet);
void server_Pong(void* packet);
void server_Resource_Pack_Response(void* packet);
void server_Known_Packs(void* packet);

// Typedef function pointer array for play state here