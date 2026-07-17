#include <Standards.hpp>
#include <network/Packet.hpp>
#include <network/packets/Handshake.hpp>
#include <network/packets/Status.hpp>
#include <network/packets/Login.hpp>
#include <network/packets/Config.hpp>
#include <network/packets/Play.hpp>
#include <Console.hpp>
#include <memory>

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
    PlayVec.resize(PLAY_SIZE);
    // Build vectors of vectors for each state
    // All packets must be pushed in the correct order
    HandshakeVec[0] = std::make_shared<Handshake_p>();

    StatusVec[0] = std::make_shared<Status_Request_p>();
    StatusVec[1] = std::make_shared<Ping_Request_status_p>();

    LoginVec[0] = std::make_shared<Login_Start_p>();
    LoginVec[1] = std::make_shared<Encryption_Response_p>();
    LoginVec[2] = std::make_shared<Login_Plugin_Response_p>();
    LoginVec[3] = std::make_shared<Login_Acknowledge_p>();
    LoginVec[4] = std::make_shared<Cookie_Response_login_p>();

    ConfigVec[0] = std::make_shared<Client_Information_config_p>();
    ConfigVec[1] = std::make_shared<Cookie_Response_config_p>();
    ConfigVec[2] = std::make_shared<Serverbound_Plugin_Message_config_p>();
    ConfigVec[3] = std::make_shared<Acknowledge_Finish_Config_p>();
    ConfigVec[4] = std::make_shared<Serverbound_Keep_Alive_config_p>();
    ConfigVec[5] = std::make_shared<Pong_config_p>();
    ConfigVec[6] = std::make_shared<Resource_Pack_Response_config_p>();
    ConfigVec[7] = std::make_shared<Serverbound_Known_Packs_p>();

    // Sparse: everything else is unimplemented and left null (Connection::deserializePacket
    // now checks for null handlers rather than crashing on unimplemented Play packets).
    PlayVec[0x00] = std::make_shared<Confirm_Teleportation_p>();
    PlayVec[0x18] = std::make_shared<Serverbound_Keep_Alive_play_p>();

    // Initialize the registry with packet instances
    Incoming_Registry[0] = HandshakeVec;
    Incoming_Registry[1] = StatusVec;
    Incoming_Registry[2] = LoginVec;
    Incoming_Registry[3] = ConfigVec;
    Incoming_Registry[4] = PlayVec;
    #ifdef DEBUG
        Console::getConsole().Entry("Packet_Registry::initializeRegistry(): Packet registry initialized successfully.");
    #endif
}