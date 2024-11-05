#include <Standards.hpp>
#include <Command.hpp>

Command::Command() {

}
Command::~Command() {

}

int Command::execute() {
    return 0;
}

string Command::getName() { return name; }

string Command::getDesc() { return desc; }