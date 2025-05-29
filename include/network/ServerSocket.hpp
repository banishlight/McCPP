#pragma once
#include <Standards.hpp>
#include <network/Socket.hpp>



class ServerSocket {
    public:
        ServerSocket(const string ip, const string port);
        ~ServerSocket();
        Socket Accept();
        bool isValid();
    protected:
        #ifdef LINUX
            int _fd = -1;
            bool blocking = true;
        #endif
        #ifdef WINDOWS
            
        #endif
};