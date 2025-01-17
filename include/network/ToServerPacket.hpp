#pragma once
#include <Standards.hpp>
#include <network/Connection.hpp>

// Handshaking State
void server_Handshake(Connection conn, void* data);

// Status State
void server_Status_Request(Connection conn);
void server_Ping_Request(Connection conn, void* data);

// Login State
void server_Login_Start(Connection conn, void* data);
void server_Encryption_Response(Connection conn, void* data);
void server_Login_Plugin_Response(Connection conn, void* data);
void server_Login_Acknowledged(Connection conn);
void server_Cookie_Reponse_login(Connection conn, void* data);

// Configuration State
void server_Client_Information(Connection conn, void* data);
void server_Cookie_Response(Connection conn, void* data);
void server_Plugin_Message(Connection conn, void* data);
void server_Acknowledge_Finish_Config(Connection conn, void* data);
void server_Keep_Alive(Connection conn, void* data);
void server_Pong(Connection conn, void* data);
void server_Resource_Pack_Response(Connection conn, void* data);
void server_Known_Packs(Connection conn, void* data);

// Typedef function pointer array for play state here