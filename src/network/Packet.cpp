#include <Standards.hpp>
#include <Properties.hpp>
#include <network/Packet.hpp>
#include <network/PacketUtils.hpp>
#include <network/Connection.hpp>
#include <network/Crypto.hpp>
#include <Console.hpp>
#include <memory>
#include <vector>
#include <lib/json.hpp>
using json = nlohmann::json;

// Serverbound Handshake packet 0x00
void Handshake_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // int total_size = varIntDeserialize(in_buff);
    // in_buff.erase(in_buff.begin(), in_buff.begin() + getVarIntSize(total_size));

    // Remove the packet ID from the buffer
    // in_buff.erase(in_buff.begin(), in_buff.begin() + 1);

    int protocolVersion = varIntDeserialize(in_buff);
    in_buff.erase(in_buff.begin(), in_buff.begin() + getVarIntSize(protocolVersion));

    string serverAddress = deserializeString(in_buff);
    unsigned short int port = (static_cast<uint16_t>(in_buff[0]) << 8) | static_cast<uint16_t>(in_buff[1]);
    in_buff.erase(in_buff.begin(), in_buff.begin() + 2);
    
    int nextState = varIntDeserialize(in_buff);
    cont.connection.setState(static_cast<ConnectionState>(nextState));
    Console::getConsole().Entry("ip received: " + serverAddress + ":" + std::to_string(port));
}

// Clientbound Status packet 0x00
int Status_Response_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // Create the JSON response with server status information
    json status_json = {
        {"version", {
            {"name", SERVER_VERSION},
            {"protocol", std::stoi(PROTOCOL_VERSION)}
        }},
        {"players", {
            {"max", Properties::getProperties().max_players},
            {"online", 0},  // TODO: Fetch actual online player count
            {"sample", json::array()}  // TODO: Fetch actual player sample
        }},
        {"description", {
            {"text", Properties::getProperties().getMotd()}
        }},
        {"enforcesSecureChat", false},
        {"previewsChat", false}
    };
    string json_str = status_json.dump();
    std::vector<Byte> lengthBytes = varIntSerialize(static_cast<int>(json_str.size()));
    std::vector<Byte> packetID = varIntSerialize(getID());
    std::vector<Byte> serial_json = serializeString(json_str);
    out_buff.insert(out_buff.end(), lengthBytes.begin(), lengthBytes.end());
    out_buff.insert(out_buff.end(), packetID.begin(), packetID.end());
    out_buff.insert(out_buff.end(), serial_json.begin(), serial_json.end());
    return 0;
}

// Clientbound Status packet 0x01
int Pong_Response_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // Serialize the current timestamp as a Long
    long timestamp = cont.connection.getPing();
    std::vector<Byte> packetID = varIntSerialize(getID());
    std::vector<Byte> timestampBytes;
    timestampBytes.reserve(8);
    // Faster than a for loop
    timestampBytes[0] = static_cast<Byte>(timestamp & 0xFF);
    timestampBytes[1] = static_cast<Byte>((timestamp >> 8) & 0xFF);
    timestampBytes[2] = static_cast<Byte>((timestamp >> 16) & 0xFF);
    timestampBytes[3] = static_cast<Byte>((timestamp >> 24) & 0xFF);
    timestampBytes[4] = static_cast<Byte>((timestamp >> 32) & 0xFF);
    timestampBytes[5] = static_cast<Byte>((timestamp >> 40) & 0xFF);
    timestampBytes[6] = static_cast<Byte>((timestamp >> 48) & 0xFF);
    timestampBytes[7] = static_cast<Byte>((timestamp >> 56) & 0xFF);
    std::vector<Byte> lengthBytes = varIntSerialize(static_cast<int>(timestampBytes.size() + packetID.size()));
    out_buff.insert(out_buff.end(), lengthBytes.begin(), lengthBytes.end());
    out_buff.insert(out_buff.end(), packetID.begin(), packetID.end());
    out_buff.insert(out_buff.end(), timestampBytes.begin(), timestampBytes.end());
    return 0;
}

// Serverbound Status packet 0x00
void Status_Request_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // No Data to deserialize
    std::shared_ptr<Outgoing_Packet> responsePacket = std::make_shared<Status_Response_p>();
    cont.connection.addPacket(responsePacket);
}

void Ping_Request_status_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    long timestamp = in_buff[0] | 
                    (static_cast<long>(in_buff[1]) << 8) |
                    (static_cast<long>(in_buff[2]) << 16) | 
                    (static_cast<long>(in_buff[3]) << 24) |
                    (static_cast<long>(in_buff[4]) << 32) |
                    (static_cast<long>(in_buff[5]) << 40) |
                    (static_cast<long>(in_buff[6]) << 48) |
                    (static_cast<long>(in_buff[7]) << 56);
    // respond with a Pong_Response_p packet
    cont.connection.setPing(timestamp);
}

int Disconnect_login_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // Create JSON text component for the disconnect reason
    json reason_json = {
        {"text", "Disconnected by server."}  // Example reason text
    };
    // Convert JSON to string
    string reason_str = reason_json.dump();
    // Serialize the string length as VarInt
    std::vector<Byte> lengthBytes = varIntSerialize(static_cast<int>(reason_str.size()) + 1);
    std::vector<Byte> packetID = varIntSerialize(getID());
    std::vector<Byte> serial_json = serializeString(reason_str);
    out_buff.insert(out_buff.end(), lengthBytes.begin(), lengthBytes.end());
    out_buff.insert(out_buff.end(), packetID.begin(), packetID.end());
    out_buff.insert(out_buff.end(), serial_json.begin(), serial_json.end());
    return 0;
}

int Encryption_Request_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // Server ID (string 20)
    // Public key (Prefixed array of byte)
    // Verify Token (Prefixed array of byte)
    // Should authenticate (Boolean)
    string server_id = "";
    std::vector<Byte> pub_key = generatePublicKey();
    std::vector<Byte> verify_token = generateVerifyToken();
    pub_key = serializePrefixedArray(pub_key);
    verify_token = serializePrefixedArray(verify_token);
    Byte authenticate = 0x01; // 0x00 false, 0x01 true
    std::vector<Byte> data = serializeString(server_id);
    data.insert(data.end(), pub_key.begin(), pub_key.end());
    data.insert(data.end(), verify_token.begin(), verify_token.end());
    data.push_back(authenticate);
    std::vector<Byte> packetID = varIntSerialize(getID());
    std::vector<Byte> size = varIntSerialize(data.size() + 1);
    out_buff.insert(out_buff.end(), size.begin(), size.end());
    out_buff.insert(out_buff.end(), packetID.begin(), packetID.end());
    out_buff.insert(out_buff.end(), data.begin(), data.end());
    return 0;
}

int Login_Success_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {

    return 0;
}

int Set_Compression_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Login_Plugin_Request_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Cookie_Request_login_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

void Login_Start_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

void Encryption_Response_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

void Login_Plugin_Response_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

void Login_Acknowledge_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

void Cookie_Response_login_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

int Cookie_Request_config_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Clientbound_Plugin_Message_config_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Disconnect_config_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Finish_Config_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Clientbound_Keep_Alive_config_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Ping_config_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Reset_Chat_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Registry_Data_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Remove_Resource_Pack_config_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Add_Resource_Pack_config_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Store_Cookie_config_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Transfer_config_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Feature_Flags_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Update_Tags_config_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Clientbound_Known_Packs_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Custom_Report_Details_config_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

int Server_Links_config_p::serialize(std::vector<Byte>& out_buff, PacketContext& cont) const {
    // TODO Implementation here
    return 0;
}

void Client_Information_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

void Cookie_Response_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

void Serverbound_Plugin_Message_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

void Acknowledge_Finish_Config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

void Serverbound_Keep_Alive_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

void Pong_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

void Resource_Pack_Response_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

void Serverbound_Known_Packs_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

Packet_Registry::Packet_Registry() {
    initializeRegistry();
}

Packet_Registry& Packet_Registry::getInstance() {
    static Packet_Registry instance;
    return instance;
}

std::shared_ptr<Incoming_Packet> Packet_Registry::fetchIncomingPacket(ConnectionState state, int packetID) {
    if (state >= Incoming_Registry.size() || 
            static_cast<std::size_t>(packetID) >= Incoming_Registry[state].size() ||
            !Incoming_Registry[state][packetID]) {
            return nullptr;
        }
    return Incoming_Registry[state][packetID];
}

void Packet_Registry::initializeRegistry() {
    #ifdef DEBUG
        Console::getConsole().Entry("Packet_Registry::initializeRegistry(): Initializing packet registry.");    
    #endif
    // Perform Resizes to enforce correct size
    Incoming_Registry.resize(INC_SIZE);
    HandshakeVec.resize(HANDSHAKE_SIZE);
    StatusVec.resize(STATUS_SIZE);
    LoginVec.resize(LOGIN_SIZE);
    ConfigVec.resize(CONFIG_SIZE);
    // TODO PlayVec.resize(PLAY_SIZE);
    // Build vectors of vectors for each state
    // All packets must be pushed in the correct order
    HandshakeVec.push_back(std::make_shared<Handshake_p>());

    StatusVec.push_back(std::make_shared<Status_Request_p>());
    StatusVec.push_back(std::make_shared<Ping_Request_status_p>());
    
    LoginVec.push_back(std::make_shared<Login_Start_p>());
    LoginVec.push_back(std::make_shared<Encryption_Response_p>());
    LoginVec.push_back(std::make_shared<Login_Plugin_Response_p>());
    LoginVec.push_back(std::make_shared<Login_Acknowledge_p>());
    LoginVec.push_back(std::make_shared<Cookie_Response_login_p>());

    ConfigVec.push_back(std::make_shared<Client_Information_config_p>());
    ConfigVec.push_back(std::make_shared<Cookie_Response_config_p>());
    ConfigVec.push_back(std::make_shared<Serverbound_Plugin_Message_config_p>());
    ConfigVec.push_back(std::make_shared<Acknowledge_Finish_Config_p>());
    ConfigVec.push_back(std::make_shared<Serverbound_Keep_Alive_config_p>());
    ConfigVec.push_back(std::make_shared<Pong_config_p>());
    ConfigVec.push_back(std::make_shared<Resource_Pack_Response_config_p>());
    ConfigVec.push_back(std::make_shared<Serverbound_Known_Packs_p>());

    // Initialize the registry with packet instances
    Incoming_Registry.push_back(HandshakeVec);
    Incoming_Registry.push_back(StatusVec);
    Incoming_Registry.push_back(LoginVec);
    Incoming_Registry.push_back(ConfigVec);
    Incoming_Registry.push_back(PlayVec);
    #ifdef DEBUG
        Console::getConsole().Entry("Packet_Registry::initializeRegistry(): Packet registry initialized successfully.");
    #endif
}
