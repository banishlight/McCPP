#include "../Standards.hpp"


// Handshaking state
void server_Handshake(VarInt protocol_version, string server_address, unsigned short server_port, VarInt next_state);

// Status state
void server_Status_Request(void);
void server_Ping_Request(long payload);

// Login State
void server_Login_Start(string name, UUID player_uuid);
void server_Encryption_Response(VarInt shared_secret_length, ByteArray shared_secret, VarInt verify_token_length, ByteArray verify_token);
void server_Login_Plugin_Response(VarInt message_id, bool success, ByteArray data);
void server_Login_Acknowledged(void);
void server_Cookie_Reponse_login(int key, bool has_payload, VarInt payload_length, ByteArray payload);
