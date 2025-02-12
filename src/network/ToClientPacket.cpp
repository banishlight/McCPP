#include <network/ToClientPacket.hpp>
#include <Standards.hpp>
#include <network/VarIntLong.hpp>
#include <Console.hpp>
#include <lib/json.hpp>
using json = nlohmann::json;

void client_Status_Reponse(Connection conn, json json) {
    #warning "client_Status_Reponse() not implemented"
    Console::getConsole().Error("client_Status_Reponse() not implemented");
}

void client_Ping_Response(Connection conn, Int64 payload) {
    
}

void client_Disconnect_login(Connection conn, json reason) {

}

void client_Encryption_Request(string server_id, VarInt public_key_length, ByteArray public_key, VarInt verify_token_length, ByteArray verify_token, bool should_auth) {
    
}

void client_Login_Success(UUID uuid, string username, VarInt num_of_prop, struct LoginProperty properties[], bool strict_error_handling) {
    
}

void client_Set_Compression(VarInt threshold) {
    
}

void client_Login_Plugin_Request(VarInt message_id, Identifier channel, ByteArray data) {
    
}

void client_Cookie_Request(Identifier key) {
    
}

void client_CookieRequest_config(Identifier key) {
    
}

void client_Plugin_Message(Identifier channel, ByteArray data) {
    
}

void client_Disconnect_config(TextComponent reason) {
    
}

void client_Finish_Configuration(void) {
    
}

void client_Keep_Alive_config(long keepAliveID) {
    
}

void client_Ping_config(int ID) {
    
}

void client_Reset_Chat(void) {
    
}

void client_Registry_Data(Identifier reg_id, VarInt entry_count, Identifier* entry_id, bool* has_data, void* nbt) {
    
}

void client_Remove_resource_Pack(bool has_uuid, UUID uuid) {
    
}

void client_Add_Resource_Pack(UUID uuid, string url, string hash, bool forced, bool has_prompt, TextComponent prompt) {
    
}

void client_Store_Cookie(Identifier key, VarInt payload_len, ByteArray payload) {
    
}

void client_Transfer(string host, VarInt port) {
    
}

void client_Feature_Flags(VarInt total_feat, Identifier* feat_flags) {
    
}

void client_Update_Tags(VarInt len, Identifier registry, void* tag_arr) {
    
}

void client_Known_Packs(VarInt count, string name_space[], string id[], string ver[]) {
    
}

void client_Custom_Report_Detail(VarInt count, string title[], string desc[]) {
    
}

void client_ServerLink(VarInt count, bool is_buitin[], VarInt label[], string URL[]) {
    
}