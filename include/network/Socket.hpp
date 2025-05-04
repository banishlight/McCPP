#pragma once
#include <Standards.hpp>


class Socket {
    public:
        Socket();
        ~Socket();

        int getSocketFD() const;
        void setSocketFD(int socket_fd);

        bool isValid() const;

        void close();
        bool isClosed() const;
}