#include <Standards.hpp>
#include <Properties.hpp>
#include <network/packets/Status.hpp>
#include <network/PacketUtils.hpp>
#include <network/Connection.hpp>
#include <Console.hpp>
#include <memory>
#include <lib/json.hpp>
using json = nlohmann::json;

Status_Response_p::Status_Response_p(int threshold) {
    _threshold = threshold;
}

// Clientbound Status packet 0x00
std::vector<Byte> Status_Response_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Status_Response_p::serialize(): Sending.");
    #endif
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
    #ifdef DEBUG
        Console::getConsole().Entry("Pong_Response_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet;
    std::vector<Byte> packetID = varIntSerialize(getID());
    std::vector<Byte> timestampBytes;
    timestampBytes.resize(8);
    // Timestamp is formatted Big-endian byte order (MSB first)
    // Faster than a for loop
    timestampBytes[0] = static_cast<Byte>((_timestamp >> 56) & 0xFF);
    timestampBytes[1] = static_cast<Byte>((_timestamp >> 48) & 0xFF);
    timestampBytes[2] = static_cast<Byte>((_timestamp >> 40) & 0xFF);
    timestampBytes[3] = static_cast<Byte>((_timestamp >> 32) & 0xFF);
    timestampBytes[4] = static_cast<Byte>((_timestamp >> 24) & 0xFF);
    timestampBytes[5] = static_cast<Byte>((_timestamp >> 16) & 0xFF);
    timestampBytes[6] = static_cast<Byte>((_timestamp >> 8) & 0xFF);
    timestampBytes[7] = static_cast<Byte>(_timestamp & 0xFF);
    packet = assemblePacket(getID(), _threshold, timestampBytes);
    return packet;
}

// Serverbound Status packet 0x00
void Status_Request_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Status_Request_p::deserialize(): Received.");
    #endif
    // No Data to deserialize
    // TODO: NEED TO PASS THE PROPER PACKET COMPRESSION
    std::shared_ptr<Outgoing_Packet> responsePacket = std::make_shared<Status_Response_p>(cont.connection.getCompressionThreshold());
    cont.connection.addPacket(responsePacket);
}

void Ping_Request_status_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Ping_Request_status_p::deserialize(): Received.");
    #endif
    long timestamp = in_buff[0] |
                    (static_cast<long>(in_buff[1]) << 8) |
                    (static_cast<long>(in_buff[2]) << 16) |
                    (static_cast<long>(in_buff[3]) << 24) |
                    (static_cast<long>(in_buff[4]) << 32) |
                    (static_cast<long>(in_buff[5]) << 40) |
                    (static_cast<long>(in_buff[6]) << 48) |
                    (static_cast<long>(in_buff[7]) << 56);
    // respond with a Pong_Response_p packet
    int threshold = cont.connection.getCompressionThreshold();
    std::shared_ptr<Outgoing_Packet> responsePacket = std::make_shared<Pong_Response_p>(threshold, timestamp);
    cont.connection.addPacket(responsePacket);
}