#include <Standards.hpp>

class Packet {
    virtual ~Packet() = default;
    virtual int getID() const = 0;
    virtual Connection_State getState() const = 0;
    virtual int serialize(void*& buffer) const = 0;
    virtual int deserialize(const void* buffer) = 0;
};

class HandshakePacket : public Packet {
    private:
        int protocolVersion;
        string serverAddress;
        int serverPort;
        Connection_State nextState;
    public:
        // HandshakePacket(int protocolVersion, const std::string& serverAddress, int serverPort, Connection_State nextState);
        // ~HandshakePacket() override = default;
        int getID() const override;
        Connection_State getState() const override;
        int serialize(void*& buffer) const override;
        int deserialize(const void* buffer) override;
};