#include <Standards.hpp>
#include <network/Packet.hpp>
#include <network/PacketUtils.hpp>
#include <network/Connection.hpp>
#include <Console.hpp>
#include <memory>
#include <vector>
#include <lib/json.hpp>
using json = nlohmann::json;

// Serverbound Handshake packet 0x00
int Handshake_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    int protocolVersion = varIntDeserialize(in_buff);
    int i = getVarIntSize(protocolVersion);
    in_buff.erase(in_buff.begin(), in_buff.begin() + i);
    string serverAddress = deserializeString(in_buff);
    short int port = (static_cast<uint16_t>(in_buff[0]) << 8) | static_cast<uint16_t>(in_buff[1]);
    in_buff.erase(in_buff.begin(), in_buff.begin() + 2);
    int nextState = varIntDeserialize(in_buff);
    cont.connection.setState(static_cast<ConnectionState>(nextState));
    Console::getConsole().Entry("ip received: " + serverAddress + ":" + std::to_string(port));
    return 0;
}

// Clientbound Status packet 0x00
int Status_Response_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

// Clientbound Status packet 0x01
int Pong_Response_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

// Serverbound Status packet 0x00
int Status_Request_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
    return 0;
}

int Ping_Request_status_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
    return 0;
}

int Disconnect_login_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Encryption_Request_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Login_Success_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Set_Compression_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Login_Plugin_Request_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Cookie_Request_login_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Login_Start_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
    return 0;
}

int Encryption_Response_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
    return 0;
}

int Login_Plugin_Response_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
    return 0;
}

int Login_Acknowledge_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
    return 0;
}

int Cookie_Response_login_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
    return 0;
}

int Cookie_Request_config_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Clientbound_Plugin_Message_config_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Disconnect_config_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Finish_Config_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Clientbound_Keep_Alive_config_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Ping_config_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Reset_Chat_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Registry_Data_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Remove_Resource_Pack_config_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Add_Resource_Pack_config_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Store_Cookie_config_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Transfer_config_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Feature_Flags_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Update_Tags_config_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Clientbound_Known_Packs_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Custom_Report_Details_config_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Server_Links_config_p::serialize(std::vector<Byte>& out_buff) const {
    // TODO Implementation here
    return 0;
}

int Client_Information_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
    return 0;
}

int Cookie_Response_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
    return 0;
}

int Serverbound_Plugin_Message_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
    return 0;
}

int Acknowledge_Finish_Config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
    return 0;
}

int Serverbound_Keep_Alive_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
    return 0;
}

int Pong_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
    return 0;
}

int Resource_Pack_Response_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
    return 0;
}

int Serverbound_Known_Packs_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
    return 0;
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
