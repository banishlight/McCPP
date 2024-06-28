#include "../../include/network/Connection.hpp"

class Connection {
    public:
        Connection() { 
            Connection_State myState = Handshake;
        }

        ~Connection() { 

        }

    private:
    enum Connection_State {
        Handshake,
        Status,
        Login,
        Play,
        Closed
    };
};