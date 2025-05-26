#include <Standards.hpp>
#include <network/Socket.hpp>



class ServerSocket : public Socket {
    public:
        ServerSocket(const string ip, const string port);
        ~ServerSocket();
        Socket Accept();
    protected:
        #ifdef LINUX
            int fd = -1;
            bool blocking = true;
        #endif
        #ifdef WINDOWS
            
        #endif
};