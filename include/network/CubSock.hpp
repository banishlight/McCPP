#include <Standards.hpp>

// Will be using SOCK_STREAM sockets
class CubSock {
    public:
        CubSock();
        ~CubSock();
        void Bind();
        void Accept();
        void Listen();
        void Connect();
    private:

};