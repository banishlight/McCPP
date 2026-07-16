#include <Standards.hpp>
#include <network/packets/Handshake.hpp>
#include <network/PacketUtils.hpp>
#include <network/Connection.hpp>
#include <Console.hpp>

// Serverbound Handshake packet 0x00
void Handshake_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Handshake_p::deserialize(): Received.");
    #endif
    int protocolVersion = varIntDeserialize(in_buff);
    in_buff.erase(in_buff.begin(), in_buff.begin() + getVarIntSize(protocolVersion));

    string serverAddress = deserializeString(in_buff);
    unsigned short int port = (static_cast<uint16_t>(in_buff[0]) << 8) | static_cast<uint16_t>(in_buff[1]);
    in_buff.erase(in_buff.begin(), in_buff.begin() + 2);

    int nextState = varIntDeserialize(in_buff);
    // cont.connection.setState(static_cast<ConnectionState>(nextState));
    switch (nextState) {
        case 1: // Status
            cont.connection.setState(ConnectionState::Status);
            #ifdef DEBUG
            Console::getConsole().Entry("Handshake received, switching to Status state.");
            #endif
            break;
        case 2: // Login
            cont.connection.setState(ConnectionState::Login);
            #ifdef DEBUG
            Console::getConsole().Entry("Handshake received, switching to Login state.");
            #endif
            break;
        default:
            #ifdef DEBUG
            Console::getConsole().Error("Invalid next state in Handshake packet: " + std::to_string(nextState));
            #endif
            return; // Invalid state, do not proceed
    }
    Console::getConsole().Entry("ip received: " + serverAddress + ":" + std::to_string(port));
}