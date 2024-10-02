#include <iostream>
#include <vector>
#include <pthread.h>
#include <Standards.hpp>
#include <Properties.hpp>
#include <network/Connection.hpp>

int main()
{
    std::cout << "A C++ Minecraft Server" << std::endl;
    //const char* ip = "15.235.13.44";
    // ProcessHandshake(764,25565,ip);

    // Initialize Configs
    Properties* myProperties = new Properties();

    // Initialize World

    // Create worker threads

    // Begin Connection handler
    std::vector<Connection> connections;
    connections.reserve(myProperties->max_players);


    return 0;
}

