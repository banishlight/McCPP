#pragma once
#include <Standards.hpp>
#include <lib/json.hpp>
using json = nlohmann::json;

class Packet {
    public:
        virtual ~Packet() = default;
        virtual Connection_State getState() const = 0;
        virtual int getID() const = 0;
};

class Incoming_Packet : public virtual Packet {
    public:
        virtual int deserialize(const void* in_buff) = 0;
};
class Outgoing_Packet : public virtual Packet {
    public:
        virtual int serialize(void* out_buffer) const = 0;
};


class Handshake_Packet : public virtual Packet {
    public:
        Connection_State getState() const override { return ConnectionState::Handshake; }
};
class Status_Packet : public virtual Packet {
    public:
        Connection_State getState() const override { return ConnectionState::Status; }
};
class Login_Packet : public virtual Packet {
    public:
        Connection_State getState() const override { return ConnectionState::Login; }
};
class Config_Packet : public virtual Packet {
    public:
        Connection_State getState() const override { return ConnectionState::Config; }
};
class Play_Packet : public virtual Packet {
    public:
        Connection_State getState() const override { return ConnectionState::Play; }
};

// **Handshake Packets**

class Handshake_p : public Handshake_Packet, public Incoming_Packet {
    // Standard format for writing Packets?
    public:
        int getID() const override { return _PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
	    static int constexpr _PACKET_ID = 0x00;
}

// **Status Packets**

class Status_Response_p : public Status_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x00;
}
class Pong_Response_p : public Status_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x01;
}

class Status_Request_p : public Status_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x00;
}
class Ping_Request_status_p : public Status_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x01;
}

// **Login Packets**

class Disconnect_login_p : public Login_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x00;
}
class Encryption_Response_p : public Login_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x01;
}
class Login_Success_p : public Login_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x02;
}
class Set_Compression_p : public Login_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x03;
}
class Login_Plugin_Request_p : public Login_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x04;
}
class Cookie_Request_login_p : public Login_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x05;
}

class Login_Start_p : public Login_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x00;
}
class Encryption_Response_p : public Login_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x01;
}
class Login_Plugin_Response_p : public Login_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x02;
}
class Login_Acknowledge_p : public Login_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x03;
}
class Cookie_Response_login_p : public Login_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x04;
}

// **Config Packets**

class Cookie_Request_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x00;
}
class Clientbound_Plugin_Message_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x01;
}
class Disconnect_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x02;
}
class Finish_Config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x03;
}
class Clientbound_Keep_Alive_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x04;
}
class Ping_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x05;
}
class Reset_Chat_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x06;
}
class Registry_Data_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x07;
}
class Remove_Resource_Pack_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x08;
}
class Add_Resource_Pack_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x09;
}
class Store_Cookie_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x0A;
}
class Transfer_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x0B;
}
class Feature_Flags_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x0C;
}
class Update_Tags_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x0D;
}
class Clientbound_Known_Packs_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x0E;
}
class Custom_Report_Details_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x0F;
}
class Server_Links_config_p : public Config_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        static int constexpr _PACKET_ID = 0x10;
}

class Client_Information_config_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x00;
}
class Cookie_Response_config_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x01;
}
class Serverbound_Plugin_Message_config_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return PACK_PACKET_IDET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x02;
}
class Acknowledge_Finish_Config_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x03;
}
class Serverbound_Keep_Alive_config_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x04;
}
class Pong_config_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x05;
}
class Resource_Pack_Response_config_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x06;
}
class Serverbound_Known_Packs_p : public Config_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        static int constexpr _PACKET_ID = 0x07;
}

// **Play Packets**