#include <network/ToServerPacket.hpp>
#include <network/ToClientPacket.hpp>
#include <Standards.hpp>
#include <network/VarIntLong.hpp>
#include <network/Connection.hpp>
#include <Console.hpp>
#include <network/readData.hpp>
#include <Properties.hpp>
#include <lib/json.hpp>
using json = nlohmann::json;

// Handshake State

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
    string maxplayers = Properties::getProperties().getMaxPlayers();
    string motd = Properties::getProperties().getMotd();
    json status_json = {
        {"version", {
            {"name", SERVER_VERSION},
            {"protocol", PROTOCOL_VERSION}
        }},
        {"players", {
            {"max", maxplayers},
            {"online", 0}
        }},
        {"description", {
            {"text", motd}
        }}
    };
    client_Status_Reponse(conn, status_json);
}

void server_Ping_Request(Connection conn, void* data) {
    Int64 timestamp;
    readData(data, timestamp);
    client_Ping_Response(conn, timestamp);
}

// Login State

void server_Login_Start(Connection conn, void* data) {
    // string name, UUID player_uuid
    string name;
    readData(data, name, 16);
    Int64 first;
    readData(data, first);
    Int64 second;
    readData(data, second);
    // TODO do something with name and UUID
}

// VarInt shared_secret_length, ByteArray shared_secret, VarInt verify_token_length, ByteArray verify_token
void server_Encryption_Response(Connection conn, void* data) {
    
}

// VarInt message_id, bool success, ByteArray data
void server_Login_Plugin_Response(Connection conn, void* data) {

}

void server_Login_Acknowledged(Connection conn) {
    conn.setState(Connection::Config);
}

// Int32 key, bool has_payload, VarInt payload_length, ByteArray payload
void server_Cookie_Reponse_login(Connection conn, void* data) {

}

// Configuration State

// string locale, Byte view_d, VarInt chat_mode, bool chat_colour, Int16 display_skin_parts, VarInt main_hand, bool text_filter, bool server_listings
void server_Client_Information(Connection conn, void* data) {

}

// Identifier key, bool has_payload, VarInt paylod_len, ByteArray payload
void server_Cookie_Response(Connection conn, void* data) {

}

// Identifier channel, ByteArray data
void server_Plugin_Message(Connection conn, void* data) {

}

void server_Acknowledge_Finish_Config(Connection conn) {
    conn.setState(Connection::Play);
}

// Int64 id
void server_Keep_Alive(Connection conn, void* data) {

}

// Int32 id
void server_Pong(Connection conn, void* data) {

}

// UUID uuid, VarInt result
void server_Resource_Pack_Response(Connection conn, void* data) {

}

// VarInt pack_count, string* name_space, string* id, string* version
void server_Known_Packs(Connection conn, void* data) {
    
}