#include <network/ToServerPacket.hpp>
#include <Standards.hpp>
#include <network/VarIntLong.hpp>
#include <network/Connection.hpp>
#include <Console.hpp>
#include <network/readData.hpp>

// Handshake State
// VarInt protocol_version, string server_address, UInt16 server_port, VarInt next_state
void server_Handshake(Connection conn, void* data) {
    VarInt proto_ver = VarInt(data);
    string serv_add; 
    readData(data, serv_add, 255);
    UInt16 port;
    readData(data, port); 
    VarInt state = VarInt(data);
    int value = state.getValue();
    switch(value) {
        case 1: // Status
            conn.setState(Connection::Connection_State::Status);
            break;
        case 2: // Login
            conn.setState(Connection::Connection_State::Login);
            break;
        case 3: // Transfer
            // Transfer state??
            #warning "Unhandled transfer state switch in server_Handshake()"
            Console::getConsole().Error("Unhandled transfer state in handshake.");
            break;
        default:
            Console::getConsole().Error("Unknown state in handshake packet.");
    }
}

// Status State
void server_Status_Request(Connection conn) {

}

// Int64 payload
void server_Ping_Request(void* pacpacketDataket) {

}

// Login State
// string name, UUID player_uuid
void server_Login_Start(void* packetData) {

}

// VarInt shared_secret_length, ByteArray shared_secret, VarInt verify_token_length, ByteArray verify_token
void server_Encryption_Response(void* packetData) {

}

// VarInt message_id, bool success, ByteArray data
void server_Login_Plugin_Response(void* packetData) {

}

void server_Login_Acknowledged(void) {

}

// Int32 key, bool has_payload, VarInt payload_length, ByteArray payload
void server_Cookie_Reponse_login(void* packetData) {

}

// Configuration State
// string locale, Byte view_d, VarInt chat_mode, bool chat_colour, Int16 display_skin_parts, VarInt main_hand, bool text_filter, bool server_listings
void server_Client_Information(void* packetData) {

}

// Identifier key, bool has_payload, VarInt paylod_len, ByteArray payload
void server_Cookie_Response(void* packetData) {

}

// Identifier channel, ByteArray data
void server_Plugin_Message(void* packetData) {

}

void server_Acknowledge_Finish_Config(void) {

}

// Int64 id
void server_Keep_Alive(void* packet) {

}

// Int32 id
void server_Pong(void* packet) {

}

// UUID uuid, VarInt result
void server_Resource_Pack_Response(void* packet) {

}

// VarInt pack_count, string* name_space, string* id, string* version
void server_Known_Packs(void* packet) {
    
}