#include <Standards.hpp>
#include <Properties.hpp>
#include <network/packets/Login.hpp>
#include <network/packets/Config.hpp>
#include <network/PacketUtils.hpp>
#include <network/Connection.hpp>
#include <network/Crypto.hpp>
#include <network/HttpsClient.hpp>
#include <Console.hpp>
#include <memory>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <lib/json.hpp>
using json = nlohmann::json;

namespace {
    // Usernames are normally already URL-safe (alphanumeric + underscore), but
    // percent-encode defensively rather than assume the client-supplied string
    // never contains anything else.
    std::string urlEncodeUsername(const std::string& username) {
        std::ostringstream oss;
        for (unsigned char c : username) {
            if (std::isalnum(c) || c == '_' || c == '-' || c == '.') {
                oss << c;
            } else {
                oss << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
                oss << std::nouppercase << std::dec;
            }
        }
        return oss.str();
    }

    // Converts Mojang's dashless 32-hex-char UUID string into the
    // {mostSignificant, leastSignificant} long-pair representation
    // deserializeUUID/serializeUUID already use elsewhere.
    std::vector<long> uuidFromMojangHex(const std::string& hex) {
        if (hex.size() != 32) return {0, 0};
        unsigned long long msb = std::stoull(hex.substr(0, 16), nullptr, 16);
        unsigned long long lsb = std::stoull(hex.substr(16, 16), nullptr, 16);
        return {static_cast<long>(msb), static_cast<long>(lsb)};
    }
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
    #ifdef DEBUG
        Console::getConsole().Entry("Disconnect_login_p::serialize(): Sending.");
    #endif
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

Encryption_Request_p::Encryption_Request_p(int threshold, const std::string& serverId, const std::vector<Byte>& verifyToken) {
    _threshold = threshold;
    _serverId = serverId;
    _verifyToken = verifyToken;
}

std::vector<Byte> Encryption_Request_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Encryption_Request_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet;
    // Server ID (string 20)
    // Public key (Prefixed array of byte)
    // Verify Token (Prefixed array of byte)
    // Should authenticate (Boolean)
    std::vector<Byte> pub_key = generatePublicKey();
    std::vector<Byte> verify_token = serializePrefixedArray(_verifyToken);
    pub_key = serializePrefixedArray(pub_key);
    Byte authenticate = Properties::getProperties().online_mode ? 0x01 : 0x00; // must match whether Encryption_Response_p actually verifies with Mojang
    std::vector<Byte> data = serializeString(_serverId);
    data.insert(data.end(), pub_key.begin(), pub_key.end());
    data.insert(data.end(), verify_token.begin(), verify_token.end());
    data.push_back(authenticate);
    packet = assemblePacket(getID(), _threshold, data);
    return packet;
}

Login_Success_p::Login_Success_p(int threshold, const std::vector<long>& uuid, const std::string& username, const std::vector<PlayerProfileProperty>& properties) {
    _threshold = threshold;
    _uuid = uuid;
    _username = username;
    _properties = properties;
}

std::vector<Byte> Login_Success_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Login_Success_p::serialize(): Sending.");
    #endif
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
    // Strict Error Handling (Boolean): whether the client should disconnect on a packet
    // processing error rather than silently ignoring it. Present in 1.20.5+.
    packet_data.push_back(0x00);
    // Serialize the packet ID and size
    packet = assemblePacket(getID(), _threshold, packet_data);
    return packet;
}

Set_Compression_p::Set_Compression_p(int threshold, Connection& conn) : _my_conn(conn) {
    _threshold = threshold;
}

std::vector<Byte> Set_Compression_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Compression_p::serialize(): Sending.");
    #endif
    // WARNING, ACCESSES PROPERTIES
    std::vector<Byte> packet;
    // Compression threshold (VarInt)
    std::vector<Byte> threshold_bytes = varIntSerialize(Properties::getProperties().getCompressionThreshold());
    _my_conn.enableCompression();
    packet = assemblePacket(getID(), _threshold, threshold_bytes);
    return packet;
}

std::vector<Byte> Login_Plugin_Request_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Login_Plugin_Request_p::serialize(): Not Implemented.");
    #endif
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Cookie_Request_login_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Cookie_Request_login_p::serialize(): Not Implemented.");
    #endif
    // TODO Implementation here
    return std::vector<Byte>();
}

void Login_Start_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    string username;
    std::vector<long> uuid;
    // Deserialize the username (string)
    username = deserializeString(in_buff);
    // Deserialize the UUID (array of 2 long)
    uuid = deserializeUUID(in_buff);
    cont.connection.getPlayer().setUsername(username);
    cont.connection.getPlayer().setUUID(uuid);

    std::string serverId = generateServerId();
    std::vector<Byte> verifyToken = generateVerifyToken();
    cont.connection.setServerId(serverId);
    cont.connection.setVerifyToken(verifyToken);

    std::shared_ptr<Outgoing_Packet> encryptionRequestPacket = std::make_shared<Encryption_Request_p>(cont.connection.getCompressionThreshold(), serverId, verifyToken);
    cont.connection.addPacket(encryptionRequestPacket);
}

// Queues set compression packet
void Encryption_Response_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    Console::getConsole().Entry("Encryption_Response_p::deserialize(): Received.");
    std::vector<Byte> encrypted_shared_secret;
    std::vector<Byte> encrypted_verify_token;
    // Deserialize the shared secret (prefixed byte array)
    encrypted_shared_secret = deserializePrefixedArray(in_buff);
    // Deserialize the verify token (prefixed byte array)
    encrypted_verify_token = deserializePrefixedArray(in_buff);

    std::vector<Byte> decrypted_verify_token = decryptWithPrivateKey(encrypted_verify_token);
    if (!verifyToken(cont.connection.getVerifyToken(), decrypted_verify_token)) {
        Console::getConsole().Error("Encryption_Response_p::deserialize(): Verify token mismatch, disconnecting client.");
        int threshold = cont.connection.getCompressionThreshold();
        std::shared_ptr<Outgoing_Packet> disconnect = std::make_shared<Disconnect_login_p>(threshold, "Invalid verify token.");
        cont.connection.addPacket(disconnect);
        return;
    }

    // From here on, every byte on the wire (including the length prefix) is AES/CFB-8 encrypted,
    // so this must happen before any further packets are read from or written to the connection.
    std::vector<Byte> shared_secret = decryptWithPrivateKey(encrypted_shared_secret);
    cont.connection.enableEncryption(shared_secret);

    // Offline mode (server.properties online-mode=false): keep trusting the
    // client-supplied UUID/username from Login Start, same as before this
    // feature existed -- no Mojang round-trip, no real skin data.
    if (Properties::getProperties().online_mode) {
        // Online-mode verification: Mojang's join-hash must be computed over the
        // exact same public key DER sent in Encryption_Request_p -- generatePublicKey()
        // memoizes after its first call (see Crypto.hpp), so this reuses those same bytes.
        std::string serverHash = computeServerHash(cont.connection.getServerId(), shared_secret, generatePublicKey());
        std::string username = cont.connection.getPlayer().getUsername();
        HttpsClient::Response hasJoinedResp = HttpsClient::Get("sessionserver.mojang.com",
            "/session/minecraft/hasJoined?username=" + urlEncodeUsername(username) + "&serverId=" + serverHash);

        if (!hasJoinedResp.success || hasJoinedResp.body.empty()) {
            Console::getConsole().Error("Encryption_Response_p::deserialize(): Mojang authentication failed for \"" + username + "\", disconnecting.");
            int failThreshold = cont.connection.getCompressionThreshold();
            cont.connection.addPacket(std::make_shared<Disconnect_login_p>(failThreshold, "Failed to verify username!"));
            return;
        }

        try {
            json profile = json::parse(hasJoinedResp.body);
            Player& player = cont.connection.getPlayer();
            player.setUUID(uuidFromMojangHex(profile.at("id").get<std::string>()));
            player.setUsername(profile.at("name").get<std::string>());

            std::vector<PlayerProfileProperty> properties;
            if (profile.contains("properties")) {
                for (const auto& prop : profile["properties"]) {
                    PlayerProfileProperty p;
                    p.name = prop.value("name", "");
                    p.value = prop.value("value", "");
                    p.signature = prop.value("signature", "");
                    properties.push_back(p);
                }
            }
            player.setProfileProperties(properties);
        } catch (const json::exception& e) {
            Console::getConsole().Error("Encryption_Response_p::deserialize(): Malformed Mojang response, disconnecting: " + std::string(e.what()));
            int failThreshold = cont.connection.getCompressionThreshold();
            cont.connection.addPacket(std::make_shared<Disconnect_login_p>(failThreshold, "Failed to verify username!"));
            return;
        }
    }

    int threshold = cont.connection.getCompressionThreshold();
    std::shared_ptr<Outgoing_Packet> setCompressionPacket = std::make_shared<Set_Compression_p>(threshold, cont.connection);
    cont.connection.addPacket(setCompressionPacket);
    // Set_Compression_p above switches the connection to the real compression threshold once it's
    // flushed, and per protocol Login Success must already use compressed framing in that same flush.
    int postCompressionThreshold = Properties::getProperties().getCompressionThreshold();
    std::shared_ptr<Outgoing_Packet> login_success = std::make_shared<Login_Success_p>(postCompressionThreshold, cont.connection.getPlayer().getUUID(), cont.connection.getPlayer().getUsername(), cont.connection.getPlayer().getProfileProperties());
    cont.connection.addPacket(login_success);
}

void Login_Plugin_Response_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    Console::getConsole().Error("Login_Plugin_Response_p::deserialize(): Not implemented yet.");
}

// Transition to Config state
void Login_Acknowledge_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // No data
    cont.connection.setState(ConnectionState::Config);
    int threshold = cont.connection.getCompressionThreshold();
    std::shared_ptr<Outgoing_Packet> knownPacks = std::make_shared<Clientbound_Known_Packs_p>(threshold);
    cont.connection.addPacket(knownPacks);
}

void Cookie_Response_login_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    Console::getConsole().Error("Cookie_Response_login_p::deserialize(): Not implemented yet.");
}