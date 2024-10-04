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
    string myIP = Properties::getProperties().getIP();
    while(this->running) {

    }
}
