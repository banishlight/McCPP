#include <network/AcceptChild.hpp>
#include <network/Connection.hpp>
#include <Properties.hpp>
#include <string>
#include <Standards.hpp>
#include <stdexcept>

AcceptChild::AcceptChild() {
    this->listRef = ConnectionList::getList();
}

AcceptChild::~AcceptChild() {

}

void AcceptChild::begin() {
    string myIP = Properties::getProperties().server_ip;
    while(this->running) {

    }
}

string AcceptChild::getIP() {
    if (server_ip.compare("")) {
        // ERROR no ip set, throw exception
        throw std::runtime_error();
    }
    return server_ip;
}