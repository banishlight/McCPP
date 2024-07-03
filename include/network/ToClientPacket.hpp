#include "../Standards.hpp"


// Handshake State
// No Handshake Packet

// Status State
void client_Status_Reponse(string json_response);
void client_Ping_Response(long payload);

// Login State
void client_Disconnect_login(JsonTextComponent reason);
void client_Encryption_Request(string server_id, VarInt public_key_length, ByteArray public_key, VarInt verify_token_length, ByteArray verify_token, bool should_auth);
void client_Login_Success(UUID uuid, string username, VarInt num_of_prop, struct LoginProperty properties[], bool strict_error_handling); // Look into how to parameterize Property array struct
void client_Set_Compression(VarInt threshold);
void client_Login_Plugin_Request(VarInt message_id, Identifier channel, ByteArray data); 
void client_Cookie_Request(Identifier key); 

// Configuration State
void client_CookieRequest_config(Identifier key);
void client_Plugin_Message(Identifier channel, ByteArray data);
void client_Disconnect_config(TextComponent reason);
void client_Finish_Configuration(void);
void client_Keep_Alive_config(long keepAliveID);
void client_Ping_config(int ID);
void client_Reset_Chat();