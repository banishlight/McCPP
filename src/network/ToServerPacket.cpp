#include <network/ToServerPacket.hpp>
#include <Standards.hpp>
#include <network/VarIntLong.hpp>

// Handshake State
// VarInt protocol_version, string server_address, UInt16 server_port, VarInt next_state
void server_Handshake(void* packet) {

}

// Status State
void server_Status_Request(void) {

}

// Int64 payload
void server_Ping_Request(void* packet) {

}

// Login State
// string name, UUID player_uuid
void server_Login_Start(void* packet) {

}

// VarInt shared_secret_length, ByteArray shared_secret, VarInt verify_token_length, ByteArray verify_token
void server_Encryption_Response(void* packet) {

}

// VarInt message_id, bool success, ByteArray data
void server_Login_Plugin_Response(void* packet) {

}

void server_Login_Acknowledged(void) {

}

// Int32 key, bool has_payload, VarInt payload_length, ByteArray payload
void server_Cookie_Reponse_login(void* packet) {

}

// Configuration State
// string locale, Byte view_d, VarInt chat_mode, bool chat_colour, Int16 display_skin_parts, VarInt main_hand, bool text_filter, bool server_listings
void server_Client_Information(void* packet) {

}

// Identifier key, bool has_payload, VarInt paylod_len, ByteArray payload
void server_Cookie_Response(void* packet) {

}

// Identifier channel, ByteArray data
void server_Plugin_Message(void* packet) {

}

void server_Acknowledge_Finish_Config(void) {

}

// Int64 id
void server_Keep_Alive(void* packet) {

}

void server_Pong(Int32 id) {

}

void server_Resource_Pack_Response(UUID uuid, VarInt result) {

}

void server_Known_Packs(VarInt pack_count, string* name_space, string* id, string* version) {
    
}