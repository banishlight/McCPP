#include "../Standards.hpp"


// Handshaking state
void server_Handshake(VarInt protocol_version, string server_address, unsigned short server_port, VarInt next_state);

// Status state
void server_Status_Request();
void server_Ping_Request();