#pragma once
#include <Standards.hpp>
#include <network/VarIntLong.hpp>

// Handshaking State
void server_Handshake(VarInt protocol_version, string server_address, UInt16 server_port, VarInt next_state);

// Status State
void server_Status_Request(void);
void server_Ping_Request(Int64 payload);

// Login State
void server_Login_Start(string name, UUID player_uuid);
void server_Encryption_Response(VarInt shared_secret_length, ByteArray shared_secret, VarInt verify_token_length, ByteArray verify_token);
void server_Login_Plugin_Response(VarInt message_id, bool success, ByteArray data);
void server_Login_Acknowledged(void);
void server_Cookie_Reponse_login(Int32 key, bool has_payload, VarInt payload_length, ByteArray payload);

// Configuration State
void server_Client_Information(string locale, Byte view_d, VarInt chat_mode, bool chat_colour, Int16 display_skin_parts, VarInt main_hand, bool text_filter, bool server_listings);
void server_Cookie_Response(Identifier key, bool has_payload, VarInt paylod_len, ByteArray payload);
void server_Plugin_Message(Identifier channel, ByteArray data);
void server_Acknowledge_Finish_Config(void);
void server_Keep_Alive(Int64 id);
void server_Pong(Int32 id);
void server_Resource_Pack_Response(UUID uuid, VarInt result);
void server_Known_Packs(VarInt pack_count, string* name_space, string* id, string* version);

// Typedef function pointer array for play state here