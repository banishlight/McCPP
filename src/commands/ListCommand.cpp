#include <commands/ListCommand.hpp>
#include <CommandSender.hpp>
#include <network/ConnectionManager.hpp>
#include <network/Connection.hpp>
#include <sstream>

void ListCommand::execute(CommandSender& sender, const std::vector<string>& args) {
    std::vector<string> names;
    for (auto& conn : ConnectionManager::getInstance().getActiveConnections()) {
        if (!conn || conn->getState() != ConnectionState::Play) continue;
        names.push_back(conn->getPlayer().getUsername());
    }

    std::ostringstream oss;
    oss << "There are " << names.size() << " player(s) online";
    if (!names.empty()) {
        oss << ": ";
        for (size_t i = 0; i < names.size(); i++) {
            if (i > 0) oss << ", ";
            oss << names[i];
        }
    }
    sender.sendMessage(oss.str());
}

string ListCommand::getName() const {
    return "list";
}

string ListCommand::getDescription() const {
    return "Lists currently connected players";
}
