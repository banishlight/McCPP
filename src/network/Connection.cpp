#include "../../include/network/Connection.hpp"

class Connection {
    public:
    enum Connection_State {
        Handshake,
        Status,
        Login,
        Play,
        Closed
    };

        Connection() { 
            
        }

        ~Connection() { 

        }

    private:

};