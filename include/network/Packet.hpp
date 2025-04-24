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
        int getID() const override { return PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
	    int constexpr PACKET_ID = 0x00;
}

// **Status Packets**

class Status_Response_p : public Status_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        int constexpr PACKET_ID = 0x00;
}
class Pong_Response_p : public Status_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        int constexpr PACKET_ID = 0x01;
}

class Status_Request_p : public Status_Packet, public Incoming_Packet {
    public:
        int getID() const override { return PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        int constexpr PACKET_ID = 0x00;
}
class Ping_Request_status_p : public Status_Packet, public Incoming_Packet {
    public:
        int getID() const override { return PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        int constexpr PACKET_ID = 0x01;
}

// **Login Packets**

class Disconnect_login_p : public Login_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        int constexpr PACKET_ID = 0x00;
}
class Encryption_Response_p : public Login_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        int constexpr PACKET_ID = 0x01;
}
class Login_Success_p : public Login_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return PACKET_ID; }
        int serialize(void* out_buffer) const override;
    private:
        int constexpr PACKET_ID = 0x02;
}
class Set_Compression_p : public Login_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return PACKET_ID; }
        int serialize(const void* in_buff) override;
    private:
        int constexpr PACKET_ID = 0x03;
}
class Login_Plugin_Request_p : public Login_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return PACKET_ID; }
        int serialize(const void* in_buff) override;
    private:
        int constexpr PACKET_ID = 0x04;
}
class Cookie_Request_login_p : public Login_Packet, public Outgoing_Packet {
    public:
        int getID() const override { return PACKET_ID; }
        int serialize(const void* in_buff) override;
    private:
        int constexpr PACKET_ID = 0x05;
}

class Login_Start_p : public Login_Packet, public Incoming_Packet {
    public:
        int getID() const override { return PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        int constexpr PACKET_ID = 0x00;
}
class Encryption_Response_p : public Login_Packet, public Incoming_Packet {
    public:
        int getID() const override { return PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        int constexpr PACKET_ID = 0x01;
}
class Login_Plugin_Response_p : public Login_Packet, public Incoming_Packet {
    public:
        int getID() const override { return PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        int constexpr PACKET_ID = 0x02;
}
class Login_Acknowledge_p : public Login_Packet, public Incoming_Packet {
    public:
        int getID() const override { return PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        int constexpr PACKET_ID = 0x03;
}
class Cookie_Response_login_p : public Login_Packet, public Incoming_Packet {
    public:
        int getID() const override { return PACKET_ID; }
        int deserialize(const void* in_buff) override;
    private:
        int constexpr PACKET_ID = 0x04;
}

// **Config Packets**