#pragma once
#include <Standards.hpp>
#include <network/Packet.hpp>

// **Login Packets**

class Disconnect_login_p : public Login_Packet, public Outgoing_Packet {
    public:
        Disconnect_login_p(int threshold, const std::string& reason);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        std::string _reason;
        static int constexpr _PACKET_ID = 0x00;
};
class Encryption_Request_p : public Login_Packet, public Outgoing_Packet {
    public:
        Encryption_Request_p(int threshold);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x01;
};
class Login_Success_p : public Login_Packet, public Outgoing_Packet {
    public:
        Login_Success_p(int threshold, const std::vector<long>& uuid, const std::string& username);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
        struct login_properties {
            std::string name;
            std::string value;
            std::string signature;
        };
    private:
        static int constexpr _PACKET_ID = 0x02;
        std::vector<long> _uuid;
        std::string _username;
        std::vector<login_properties> _properties; // Optional properties
};
class Set_Compression_p : public Login_Packet, public Outgoing_Packet {
    public:
        Set_Compression_p(int threshold, Connection& conn);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        Connection& _my_conn;
        static int constexpr _PACKET_ID = 0x03;
};
class Login_Plugin_Request_p : public Login_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x04;
};
class Cookie_Request_login_p : public Login_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x05;
};

class Login_Start_p : public Login_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x00;
};
class Encryption_Response_p : public Login_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x01;
};
class Login_Plugin_Response_p : public Login_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x02;
};
class Login_Acknowledge_p : public Login_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x03;
};
class Cookie_Response_login_p : public Login_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x04;
};