#include <network/NetworkHandler.hpp>

NetworkHandler::NetworkHandler() {

}

NetworkHandler::~NetworkHandler() {

}

NetworkHandler& NetworkHandler::getHandler() {
    static NetworkHandler singleton;
    return singleton;
}

void NetworkHandler::close() {
    
}