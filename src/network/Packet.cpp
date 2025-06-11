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
    int protocolVersion = varIntDeserialize(in_buff);
    in_buff.erase(in_buff.begin(), in_buff.begin() + getVarIntSize(protocolVersion));

    string serverAddress = deserializeString(in_buff);
    unsigned short int port = (static_cast<uint16_t>(in_buff[0]) << 8) | static_cast<uint16_t>(in_buff[1]);
    in_buff.erase(in_buff.begin(), in_buff.begin() + 2);
    
    int nextState = varIntDeserialize(in_buff);
    cont.connection.setState(static_cast<ConnectionState>(nextState));
    Console::getConsole().Entry("ip received: " + serverAddress + ":" + std::to_string(port));
}

Status_Response_p::Status_Response_p(int threshold) {
    _threshold = threshold;
}

// Clientbound Status packet 0x00
std::vector<Byte> Status_Response_p::serialize() const {
    std::vector<Byte> packet;
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

    std::vector<Byte> serial_json = serializeString(json_str);
    packet = assemblePacket(getID(), _threshold, serial_json);
    return packet;
}

Pong_Response_p::Pong_Response_p(int threshold, long timestamp) {
    _threshold = threshold;
    _timestamp = timestamp;
}

// Clientbound Status packet 0x01
std::vector<Byte> Pong_Response_p::serialize() const {
    std::vector<Byte> packet;
    std::vector<Byte> packetID = varIntSerialize(getID());
    std::vector<Byte> timestampBytes;
    timestampBytes.reserve(8);
    // Faster than a for loop
    timestampBytes[0] = static_cast<Byte>(_timestamp & 0xFF);
    timestampBytes[1] = static_cast<Byte>((_timestamp >> 8) & 0xFF);
    timestampBytes[2] = static_cast<Byte>((_timestamp >> 16) & 0xFF);
    timestampBytes[3] = static_cast<Byte>((_timestamp >> 24) & 0xFF);
    timestampBytes[4] = static_cast<Byte>((_timestamp >> 32) & 0xFF);
    timestampBytes[5] = static_cast<Byte>((_timestamp >> 40) & 0xFF);
    timestampBytes[6] = static_cast<Byte>((_timestamp >> 48) & 0xFF);
    timestampBytes[7] = static_cast<Byte>((_timestamp >> 56) & 0xFF);
    packet = assemblePacket(getID(), _threshold, timestampBytes);
    return packet;
}

// Serverbound Status packet 0x00
void Status_Request_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // No Data to deserialize
    // TODO: NEED TO PASS THE PROPER PACKET COMPRESSION
    std::shared_ptr<Outgoing_Packet> responsePacket = std::make_shared<Status_Response_p>(cont.connection.getCompressionThreshold());
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

Disconnect_login_p::Disconnect_login_p(int threshold, const std::string& reason) {
    _threshold = threshold;
    if (reason.empty()) {
        _reason = "Disconnected by server.";
    } else {
        _reason = reason;
    }
}

std::vector<Byte> Disconnect_login_p::serialize() const {
    std::vector<Byte> packet;
    // Create JSON text component for the disconnect reason
    json reason_json = {
        {"text", _reason}  // Example reason text
    };
    // Convert JSON to string
    string reason_str = reason_json.dump();
    std::vector<Byte> serial_json = serializeString(reason_str);
    packet = assemblePacket(getID(), _threshold, serial_json);
    return packet;
}

Encryption_Request_p::Encryption_Request_p(int threshold) {
    _threshold = threshold;
}

std::vector<Byte> Encryption_Request_p::serialize() const {
    std::vector<Byte> packet;
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
    packet = assemblePacket(getID(), _threshold, data);
    return packet;
}

Login_Success_p::Login_Success_p(int threshold, const std::vector<long>& uuid, const std::string& username) {
    _threshold = threshold;
    _uuid = uuid;
    _username = username;
}

std::vector<Byte> Login_Success_p::serialize() const {
    // UUID
    // String (16)
    // Properties array (16)
        // String (64)
        // String (32767)
        // Prefixed Optional String (1024)
    std::vector<Byte> packet;
    std::vector<Byte> packet_data;

    std::vector<Byte> uuid_bytes = serializeUUID(_uuid);
    packet_data.insert(packet_data.end(), uuid_bytes.begin(), uuid_bytes.end());

    std::vector<Byte> username_bytes = serializeString(_username);
    packet_data.insert(packet_data.end(), username_bytes.begin(), username_bytes.end());
    // Serialize properties 
    std::vector<Byte> properties_size = varIntSerialize(static_cast<int>(_properties.size()));
    packet_data.insert(packet_data.end(), properties_size.begin(), properties_size.end());
    for (const auto& prop : _properties) {
        std::vector<Byte> name_bytes = serializeString(prop.name);
        std::vector<Byte> value_bytes = serializeString(prop.value);
        std::vector<Byte> signature_bytes = serializeString(prop.signature);
        packet_data.insert(packet_data.end(), name_bytes.begin(), name_bytes.end());
        packet_data.insert(packet_data.end(), value_bytes.begin(), value_bytes.end());
        // Prefixed optional string for signature (unsure if this is correct, but it seems to be the case)
        if (!prop.signature.empty()) {
            packet_data.push_back(0x01); // Bool true, signature present
            packet_data.insert(packet_data.end(), signature_bytes.begin(), signature_bytes.end());
        } else {
            packet_data.push_back(0x00); // Bool false, no signature
        }
    }
    // Serialize the packet ID and size
    packet = assemblePacket(getID(), _threshold, packet_data);
    return packet;
}

std::vector<Byte> Set_Compression_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Login_Plugin_Request_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Cookie_Request_login_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
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

std::vector<Byte> Cookie_Request_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Clientbound_Plugin_Message_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Disconnect_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Finish_Config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Clientbound_Keep_Alive_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Ping_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Reset_Chat_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Registry_Data_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Remove_Resource_Pack_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Add_Resource_Pack_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Store_Cookie_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Transfer_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Feature_Flags_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Update_Tags_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Clientbound_Known_Packs_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Custom_Report_Details_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Server_Links_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
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
