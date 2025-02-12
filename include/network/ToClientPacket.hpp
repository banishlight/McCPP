#pragma once
#include <Standards.hpp>
#include <network/Connection.hpp>
#include <network/VarIntLong.hpp>
#include <lib/json.hpp>
using json = nlohmann::json;

struct LoginProperty { // Property array in Login Success 
  string name;
  string value;
  bool is_signed;
  string signature; // only if signed is true
};

struct ConfigEntries {
  Identifier registryID;
  VarInt entryCount;
};

// Handshake State
// No Handshake Packet

// Status State
void client_Status_Reponse(Connection conn, json status_json);
void client_Ping_Response(Connection conn, Int64 payload);

// Login State
void client_Disconnect_login(Connection conn, json reason);
void client_Encryption_Request(Connection conn, string server_id, VarInt public_key_length, ByteArray public_key, VarInt verify_token_length, ByteArray verify_token, bool should_auth);
void client_Login_Success(Connection conn, UUID uuid, string username, VarInt num_of_prop, struct LoginProperty properties[], bool strict_error_handling); // Look into how to parameterize Property array struct
void client_Set_Compression(Connection conn, VarInt threshold);
void client_Login_Plugin_Request(Connection conn, VarInt message_id, Identifier channel, ByteArray data); 
void client_Cookie_Request(Connection conn, Identifier key); 

// Configuration State
void client_CookieRequest_config(Connection conn, Identifier key);
void client_Plugin_Message(Connection conn, Identifier channel, ByteArray data);
void client_Disconnect_config(Connection conn, TextComponent reason);
void client_Finish_Configuration(Connection conn);
void client_Keep_Alive_config(Connection conn, long keepAliveID);
void client_Ping_config(Connection conn, int ID);
void client_Reset_Chat(Connection conn);
void client_Registry_Data(Connection conn, Identifier reg_id, VarInt entry_count, Identifier* entry_id, bool* has_data, void* nbt); // unsure of nbt data type?
void client_Remove_resource_Pack(Connection conn, bool has_uuid, UUID uuid);
void client_Add_Resource_Pack(Connection conn, UUID uuid, string url, string hash, bool forced, bool has_prompt, TextComponent prompt);
void client_Store_Cookie(Connection conn, Identifier key, VarInt payload_len, ByteArray payload);
void client_Transfer(Connection conn, string host, VarInt port);
void client_Feature_Flags(Connection conn, VarInt total_feat, Identifier* feat_flags);
void client_Update_Tags(Connection conn, VarInt len, Identifier registry, void* tag_arr); // TODO: Create tag array data type (prob just a struct)
void client_Known_Packs(Connection conn, VarInt count, string name_space[], string id[], string ver[]);
void client_Custom_Report_Detail(Connection conn, VarInt count, string title[], string desc[]);
void client_ServerLink(Connection conn, VarInt count, bool is_buitin[], VarInt label[], string URL[]);
