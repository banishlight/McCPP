#pragma once
#include <Standards.hpp>

// Forward declartations
class Connection;

struct PacketContext {
    Connection& connection;
    // std::shared_ptr<actionProcessor> actionProcessor;
    // More?
    PacketContext(Connection& conn) : connection(conn) {}
};