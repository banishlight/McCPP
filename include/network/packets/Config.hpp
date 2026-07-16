#pragma once
#include <Standards.hpp>
#include <network/Packet.hpp>
#include <network/Nbt.hpp>

// **Config Packets**

class Cookie_Request_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x00;
};
class Clientbound_Plugin_Message_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x01;
};
class Disconnect_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x02;
};
class Finish_Config_p : public Config_Packet, public Outgoing_Packet {
    public:
        Finish_Config_p(int threshold);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x03;
};
class Clientbound_Keep_Alive_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        Clientbound_Keep_Alive_config_p(int threshold, Int64 keepAliveId);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        Int64 _keepAliveId;
        static int constexpr _PACKET_ID = 0x04;
};
class Ping_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        Ping_config_p(int threshold, Int32 pingId);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        Int32 _pingId;
        static int constexpr _PACKET_ID = 0x05;
};
class Reset_Chat_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x06;
};
class Registry_Data_p : public Config_Packet, public Outgoing_Packet {
    public:
        Registry_Data_p(int threshold, const std::string& registryId, std::vector<RegistryEntry> entries);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        std::string _registryId;
        std::vector<RegistryEntry> _entries;
        static int constexpr _PACKET_ID = 0x07;
};
class Remove_Resource_Pack_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x08;
};
class Add_Resource_Pack_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x09;
};
class Store_Cookie_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x0A;
};
class Transfer_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x0B;
};
class Feature_Flags_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x0C;
};
class Update_Tags_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        Update_Tags_config_p(int threshold);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x0D;
};
class Clientbound_Known_Packs_p : public Config_Packet, public Outgoing_Packet {
    public:
        Clientbound_Known_Packs_p(int threshold);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x0E;
};
class Custom_Report_Details_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x0F;
};
class Server_Links_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x10;
};

class Client_Information_config_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x00;
};
class Cookie_Response_config_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x01;
};
class Serverbound_Plugin_Message_config_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x02;
};
class Acknowledge_Finish_Config_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x03;
};
class Serverbound_Keep_Alive_config_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x04;
};
class Pong_config_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x05;
};
class Resource_Pack_Response_config_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x06;
};
class Serverbound_Known_Packs_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x07;
};